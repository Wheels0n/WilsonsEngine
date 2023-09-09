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

Texture2D posTex;
Texture2D normalTex;
Texture2D abeldoTex;
Texture2D specualrTex;//R:AO, G:Roughness, B:Metalness
TextureCube irradianceMap;
Texture2D dirShadowMaps[1];
//Texture2D spotShadowMaps[20];
//TextureCube omniDirShadowMaps[48];
SamplerState SampleType : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_shadowSampler : register(s2);

cbuffer CamBuffer
{
    float4 g_camPos;
};
cbuffer cbLight
{
    DirectionalLight cbDirLight[10];
    uint dirCnt;
    PointLight cbPointLight[48];
    uint pntCnt;
    SpotLight cbSpotLight[20];
    uint sptCnt;
    uint padding;
};
cbuffer DirLightMatrices
{
    matrix dirLightMatrices[10];
    uint dirLightCnt;
};
cbuffer SpotLightMatrices
{
    matrix spotLightMatrices[20];
    uint spotLightCnt;
};

static const float SMAP_SIZE = 1024.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;
static const float FAR_PLANE = 25.0f;
static const float PI = 3.14159265359;

float DistributionGGX(float3 normal, float3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, h), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = PI * denom * denom;
    
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

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0f - F0) * pow(clamp(1.0f - cosTheta, 0.0f, 1.0f), 5.0f);
}
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
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
    
    const float dx = SMAP_DX;
    float percentLit = 0.0f;

    const float2 offesets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
       float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
       float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    for (int i = 0; i < 9; ++i)
    {
        percentLit += shadowMap.SampleCmpLevelZero(shadowSampler,
                       shadowPos.xy + offesets[i], depth).r;
    }
   
    percentLit /= 9.0f;
    return percentLit;
}
float CalOmniDirShadowFactor(SamplerState shadowSampler,
                        TextureCube shadowMap, float3 fragToLight)
{
    
    const float dx = SMAP_DX;
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
        float sampledDepth = shadowMap.SampleLevel(shadowSampler, fragToLight + offesets[i], 0).r;
        sampledDepth *= FAR_PLANE;
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
    float3 F = fresnelSchlick(max(dot(h, viewDir), 0.0f), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metalness;
    
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / PI + specular) * radiance * NdotL;
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
    float3 F = fresnelSchlick(max(dot(h, viewDir), 0.0f), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metalness;
    
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / PI + specular) * radiance * NdotL;
    
}
void CalSpotLight(SpotLight L, float3 lightDir, float3 normal, float3 viewDir, 
float3 albedo, float metalness, float roughness, float3 F0,
out float3 L0)
{   
    L0 = float3(0.0f, 0.0f, 0.0f);
    
    float distance = length(lightDir);
    float att = 1.0f / (distance * distance);
    
    float theta = dot(lightDir, normalize(-L.direction)); //cos그래프 참조
    float epsilon = (L.cutOff - L.outerCutOff);
    float intensity = clamp((theta - L.outerCutOff) / epsilon, 0.0f, 1.0f);
    
    lightDir = normalize(lightDir);
    float3 h = normalize(viewDir + lightDir);
    float3 radiance = L.diffuse * att * intensity;
    
    //Cook-Torrance BRDF
    float NDF = DistributionGGX(normal, h, roughness);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);
    float3 F = fresnelSchlick(max(dot(h, viewDir), 0.0f), F0);
    
    float3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, viewDir), 0.0f) * max(dot(normal, lightDir), 0.0f) + 0.0001;
    float3 specular = numerator / denominator;
    
    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0f - metalness;
   
    float NdotL = max(dot(normal, lightDir), 0.0f);
    L0 += (kD * albedo / PI + specular) * radiance * NdotL;
}

PixelOutput main(PixelInput input)
{
    PixelOutput output;
    float4 wPos = posTex.Sample(SampleType, input.tex);
    float4 normal = normalTex.Sample(SampleType, input.tex);
    float4 albedo = abeldoTex.Sample(SampleType, input.tex);
    float4 spec = specualrTex.Sample(SampleType, input.tex);
    
    float3 occlusion = float3(spec.r, spec.r, spec.r);
    float roughness = spec.g;
    float metalness =spec.b;
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo.xyz, metalness);
    
    float3 viewDir = normalize(g_camPos.xyz - wPos.xyz);
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
    for (int i = 0; i < dirCnt; ++i)
    {
        dirShadowPos[i] = mul(wPos, dirLightMatrices[i]);
    }
    [unroll]
    for (int i = 0; i < sptCnt; ++i)
    {
        spotShadowPos[i] = mul(wPos, spotLightMatrices[i]);
    }
    
    
    float3 L;
    [unroll]
    for (int i = 0; i < dirCnt; ++i)
    {
        float3 lightDir = normalize(cbDirLight[i].position.xyz - wPos.xyz);
        CalDirectionalLight(cbDirLight[i], lightDir, normal.xyz, viewDir,
        albedo.xyz, metalness, roughness, F0, L);
        shadowFactor = CalShadowFactor(g_shadowSampler, dirShadowMaps[i], dirShadowPos[i], normal.xyz, lightDir);
        lightVal.xyz +=  shadowFactor * L;
        
    }
    [unroll]
    for (int j = 0; j < pntCnt; ++j)
    {   
        float3 lightDir = cbPointLight[j].position - wPos.xyz;
        CalPointLight(cbPointLight[j], lightDir, normal.xyz, viewDir, 
        albedo.xyz, metalness, roughness, F0, L);
        //shadowFactor = CalOmniDirShadowFactor(g_cubeShadowSampler, omniDirShadowMaps[j], -lightDir);
    
        lightVal.xyz += shadowFactor * L;
    }
    [unroll]
    for (int k = 0; k < sptCnt; ++k)
    {   
        float3 lightDir = (cbSpotLight[k].position - wPos.xyz);
        CalSpotLight(cbSpotLight[k], lightDir, normal.xyz, viewDir,
        albedo.xyz, metalness, roughness, F0, L);
        lightDir = normalize(lightDir);
        //shadowFactor = CalShadowFactor(g_shadowSampler, spotShadowMaps[k], spotShadowPos[k], normal.xyz, lightDir);
        lightVal.xyz += shadowFactor * L;
        
    }
    float3 kS = fresnelSchlick(max(dot(normal.xyz, viewDir.xyz), 0.0f), F0);
    float3 kD = 1.0f - kS;
    kD *= 1.0f - metalness;
    float3 irradiance = irradianceMap.Sample(SampleType, normal.xyz);
    float3 diffuse = irradiance * albedo.xyz;
    float3 ambient = (kD * diffuse) * occlusion;
   
    output.mainColor.xyz = lightVal.xyz+ambient;
    output.mainColor.a = 1.0f;
 
    return output;
}