struct PixelInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
struct PixelOutput
{
    float4 mainColor : SV_Target0;
};

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 position;
    float pad;
};
struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 att;
    float pad;
};
struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;

    float3 position;
    float range;

    float3 direction;
    float cutOff;
    
    float3 att;
    float outerCutOff;
};

Texture2D g_posTex;
Texture2D g_normalTex;
Texture2D g_abeldoTex;
Texture2D g_specualrTex;//R:AO, G:Roughness, B:Metalness
TextureCube g_irradianceMap;
TextureCube g_prefilterMap;
Texture2D g_brdfLUT;
Texture2D g_dirShadowMaps[1];
//Texture2D spotShadowMaps[20];
//TextureCube omniDirShadowMaps[48];
SamplerState g_sampler : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_shadowSampler : register(s2);

cbuffer CamBuffer
{
    float4 g_camPos;
};
cbuffer Light
{
    DirectionalLight g_DirLight[10];
    uint g_dirCnt;
    PointLight g_PointLight[48];
    uint g_pntCnt;
    SpotLight g_SpotLight[20];
    uint g_sptCnt;
    uint padding;
};
cbuffer DirLightMatrices
{
    matrix g_dirLightMatrices[10];
    uint g_dirLightCnt;
};
cbuffer SpotLightMatrices
{
    matrix g_spotLightMatrices[20];
    uint g_spotLightCnt;
};

static const float _SMAP_SIZE = 1024.0f;
static const float _SMAP_DX = 1.0f / _SMAP_SIZE;
static const float _FAR_PLANE = 25.0f;
static const float _PI = 3.14159265359;

float DistributionGGX(float3 normal, float3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, h), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = _PI * denom * denom;
   
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0f;
    float k = (r * r) / 8.0f;
    
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;

}

float GeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float roughness)
{
    float NdotV = max(dot(normal, viewDir), 0.0f);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{   
    return F0 + (max(float3(1.0f - roughness, 1.0f - roughness, 1.0f - roughness), F0) - F0) * 
    pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}

float CalShadowFactor(SamplerComparisonState shadowSampler,
                        Texture2D shadowMap,
                        float4 shadowPos,
                        float3 normal, float3 lightDir)
{
    shadowPos.xyz /= shadowPos.w;
    shadowPos.x = shadowPos.x * 0.5f + 0.5f;
    shadowPos.y = shadowPos.y * -0.5f + 0.5f;
    float bias =  max((0.05f * (1.0f - dot(normal, lightDir))), 0.005f);
    float depth = shadowPos.z - bias;
    
    const float dx = _SMAP_DX;
    float percentLit = 0.0f;

    const float2 offesets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
       float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
       float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(shadowSampler,
                       shadowPos.xy + offesets[i], depth).r;
    }
   
    percentLit /= 9.0f;
    return percentLit;
}
float CalOmniDirShadowFactor(SamplerState cubeShadowSampler,
                        TextureCube cubeShadowMap, float3 fragToLight)
{
    
    const float dx = _SMAP_DX;
    const float3 offesets[20] =
    {
        float3(dx, dx, dx), float3(dx, -dx, dx), float3(-dx, -dx, dx), float3(-dx, dx, dx),
        float3(dx, dx, -dx), float3(dx, -dx, -dx), float3(-dx, -dx, -dx), float3(-dx, dx, -dx),
        float3(dx, dx, 0), float3(dx, -dx, 0), float3(-dx, -dx, 0), float3(-dx, dx, 0),
        float3(dx, 0, dx), float3(-dx, 0, dx), float3(dx, 0, -dx), float3(-dx, 0, -dx),
        float3(0, dx, dx), float3(0, -dx, dx), float3(0, -dx, -dx), float3(0, dx, -dx)
    };
    
    float percentLit = 0.0f;
    float bias = 0.05f;
    float curDepth = length(fragToLight);
    [unroll]
    for (int i = 0; i < 20; ++i)
    {
        float sampledDepth = cubeShadowMap.SampleLevel(cubeShadowSampler, fragToLight + offesets[i], 0).r;
        sampledDepth *= _FAR_PLANE;
        if (sampledDepth > curDepth)
        {
            percentLit += 1.0f;
        }
        
    }
    percentLit /= 20.0f;
    return percentLit;

}
void CalDirectionalLight(DirectionalLight L, float3 lightDir, float3 normal, float3 viewDir,
float3 albedo, float metalness, float roughness, float3 F0, 
out float3 L0)
{ 
    
    L0 = float3(0.0f, 0.0f, 0.0f);
    
    float3 h = normalize(viewDir + lightDir);
    float3 radiance = L.diffuse;
    
    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F = FresnelSchlick(max(dot(h, viewDir), 0.0f), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;
    
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / _PI + specular) * radiance * NdotL;
}
void CalPointLight(PointLight L, float3 lightDir, float3 normal, float3 viewDir,
float3 albedo, float metalness, float roughness, float3 F0,
out float3 L0)
{
    L0 = float3(0.0f, 0.0f, 0.0f);
    
    float distance = length(lightDir);
    float att = 1.0f /(distance * distance);
	
    lightDir = normalize(lightDir);
    float3 h = normalize(viewDir + lightDir);
    float3 radiance = L.diffuse * att;
    
    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F = FresnelSchlickRoughness(max(dot(h, viewDir), 0.0f), F0, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;
    
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / _PI + specular) * radiance * NdotL;
    
}
void CalSpotLight(SpotLight L, float3 lightDir, float3 normal, float3 viewDir, 
float3 albedo, float metalness, float roughness, float3 F0,
out float3 L0)
{   
    L0 = float3(0.0f, 0.0f, 0.0f);
    
    float distance = length(lightDir);
    float att = 1.0f / (distance * distance);
    
    float theta = dot(lightDir, normalize(-L.direction)); //cos�׷��� ����
    float epsilon = (L.cutOff - L.outerCutOff);
    float intensity = clamp((theta - L.outerCutOff) / epsilon, 0.0f, 1.0f);
    
    lightDir = normalize(lightDir);
    float3 h = normalize(viewDir + lightDir);
    float3 radiance = L.diffuse * att * intensity;
    
    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F = FresnelSchlickRoughness(max(dot(h, viewDir), 0.0f), F0, roughness);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;
   
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / _PI + specular) * radiance * NdotL;
}

PixelOutput main(PixelInput input)
{
    PixelOutput output;
    float4 wPos = g_posTex.Sample(g_sampler, input.tex);
    float4 normal = g_normalTex.Sample(g_sampler, input.tex);
    float4 albedo = g_abeldoTex.Sample(g_sampler, input.tex);
    float4 spec = g_specualrTex.Sample(g_sampler, input.tex);
    
    float ao = spec.r;
    float roughness = spec.g;
    float metalness =spec.b;
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo.xyz, metalness);
    
    float3 viewDir = normalize(g_camPos.xyz - wPos.xyz);
    float3 reflection = reflect(-viewDir, normal.xyz);
    bool isCubeMap = length(normal.xyz) ? false : true;
    [branch]
    if (isCubeMap)
    {
        output.mainColor = albedo;
        return output;
    }

    float4 lightVal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float shadowFactor = 1.0f;
    
    float4 dirShadowPos[10];
    float4 spotShadowPos[20];
    [unroll]
    for (int i = 0; i < g_dirCnt; ++i)
    {
        dirShadowPos[i] = mul(wPos, g_dirLightMatrices[i]);
    }
    [unroll]
    for (int i = 0; i < g_sptCnt; ++i)
    {
        spotShadowPos[i] = mul(wPos, g_spotLightMatrices[i]);
    }
    
    
    float3 L;
    [unroll]
    for (int i = 0; i < g_dirCnt; ++i)
    {
        float3 lightDir = normalize(g_DirLight[i].position.xyz - wPos.xyz);
        CalDirectionalLight(g_DirLight[i], lightDir, normal.xyz, viewDir,
        albedo.xyz, metalness, roughness, F0, L);
        shadowFactor = CalShadowFactor(g_shadowSampler, g_dirShadowMaps[i], dirShadowPos[i], normal.xyz, lightDir);
        lightVal.xyz +=  shadowFactor * L;
        
    }
    [unroll]
    for (int j = 0; j < g_pntCnt; ++j)
    {   
        float3 lightDir = g_PointLight[j].position - wPos.xyz;
        CalPointLight(g_PointLight[j], lightDir, normal.xyz, viewDir, 
        albedo.xyz, metalness, roughness, F0, L);
        //shadowFactor = CalOmniDirShadowFactor(g_cubeShadowSampler, omniDirShadowMaps[j], -lightDir);
    
        lightVal.xyz += shadowFactor * L;
    }
    [unroll]
    for (int k = 0; k < g_sptCnt; ++k)
    {   
        float3 lightDir = (g_SpotLight[k].position - wPos.xyz);
        CalSpotLight(g_SpotLight[k], lightDir, normal.xyz, viewDir,
        albedo.xyz, metalness, roughness, F0, L);
        lightDir = normalize(lightDir);
        //shadowFactor = CalShadowFactor(g_shadowSampler, spotShadowMaps[k], spotShadowPos[k], normal.xyz, lightDir);
        lightVal.xyz += shadowFactor * L;
        
    }
    float3 kS = FresnelSchlickRoughness(max(dot(normal.xyz, viewDir.xyz), 0.0f), F0, roughness);
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;
    float3 irradiance = g_irradianceMap.Sample(g_sampler, normal.xyz);
    float3 diffuse = irradiance * albedo.xyz;
   
    const float MAX_REFLECTION_LOD = 4.0f;
    float3 prefilteredColor = g_prefilterMap.SampleLevel(g_cubeShadowSampler, reflection, roughness * MAX_REFLECTION_LOD).rgb;
    float2 brdfUV = float2(max(dot(normal.xyz, viewDir), 0.0f), roughness);
    float2 brdf = g_brdfLUT.Sample(g_sampler, brdfUV).rg;
    float3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);
    
    float3 ambient = (kD * diffuse+specular) * ao;
   
    output.mainColor.xyz = lightVal.xyz+ambient;
    output.mainColor.a = 1.0f;
 
    return output;
}