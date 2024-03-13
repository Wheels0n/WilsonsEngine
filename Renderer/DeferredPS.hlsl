struct PixelInput 
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
struct PixelOutput
{
    float4 mainColor : SV_Target0;
    float4 brightColor : SV_Target1;
};

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 position;
    float pad;
};
struct CubeLight
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
Texture2D g_specualrTex;
Texture2D g_reflectivityTex;
Texture2D g_SSAOTex;
//Texture2D dirShadowMaps[1];
//TextureCube CubeShadowMaps[1];
Texture2D g_spotShadowMaps[1];
TextureCube g_envMap;
SamplerState g_sampler : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_shadowSampler : register(s2);

cbuffer cbLight
{
    DirectionalLight g_DirLight[10];
    uint g_dirCnt;
    CubeLight g_CubeLight[48];
    uint g_CubeCnt;
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
cbuffer CamBuffer
{
    float4 g_camPos;
};

static const float _SMAP_SIZE = 1024.0f;
static const float _SMAP_DX = 1.0f / _SMAP_SIZE;
static const float _FAR_PLANE = 25.0f;

float CalShadowFactor(SamplerComparisonState shadowSampler,
                        Texture2D shadowMap,
                        float4 shadowPos,
                        float3 normal, float3 lightDir)
{
    shadowPos.xyz /= shadowPos.w;
    shadowPos.x = shadowPos.x * 0.5f + 0.5f;
    shadowPos.y = shadowPos.y * -0.5f + 0.5f;
    float bias = max((0.00025f * (1.0f - dot(normal, lightDir))), 0.000005f); // max((0.05f * (1.0f - dot(normal, lightDir))), 0.005f);
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
   
    return percentLit /= 9.0f;
}
float CalCubeShadowFactor(SamplerState cubeShadowSampler,
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
        float sampledDepth = cubeShadowMap.SampleLevel(cubeShadowSampler, fragToLight + offesets[i],0).r;
        sampledDepth *= _FAR_PLANE;
        if (sampledDepth > curDepth)
        {
            percentLit += 1.0f;
        }
        
    }
    return percentLit /= 20.0f;

}
void CalDirectionalLight(DirectionalLight L,
	float3 normal, float3 viewDir, float3 lightDir, float4 specularIntensity,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ambient= float4(L.ambient.rgb, 1.0f);
   
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
    [branch]
    if (diffuseFactor != 0.0f)
    {
        diffuse = diffuseFactor * (float4(L.diffuse.rgb, 1.0f));
        
        float3 h = normalize(viewDir + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), specularIntensity.w);
        specular = float4(specFactor * specularIntensity.rgb * L.specular.rgb, 1.0f);
    }
 
}
void CalCubeLight(CubeLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	
    float distance = length(lightDir);
    float att = 1.0f / dot(L.att, float3(1.0f, distance, distance * distance));
    [branch]
    if (distance > L.range)
    {
        return;
    }
	
    lightDir = normalize(lightDir);
	
    ambient = L.ambient * att;//광원이 여러 개면 누적이 되버림
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
	
    if (diffuseFactor != 0.0f)
    {
        diffuse = diffuseFactor * float4(L.diffuse.rgb, 1.0f);
        
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), specularIntensity.w);
        specular = float4(specFactor * specularIntensity.rgb * L.specular.rgb, 1.0f);
	    
        diffuse *= att;
        specular *= att;
    }
	
    
}
void CalSpotLight(SpotLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
    float distance = length(lightDir);
    float att = 1.0f / dot(L.att, float3(1.0f, distance, distance * distance));
    ambient = L.ambient * att;
    
    lightDir = normalize(lightDir);
    float theta = dot(lightDir, normalize(-L.direction));//cos그래프 참조
    float epsilon = (L.cutOff - L.outerCutOff);
    float intensity = clamp((theta - L.outerCutOff) / epsilon, 0.0f, 1.0f);
    
    float diffuseFactor = dot(lightDir, normal);
    if (diffuseFactor > 0.0f)
    {
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), specularIntensity.w);
		
        diffuse = diffuseFactor * float4(L.diffuse.rgb, 1.0f);
        specular = float4(specFactor * specularIntensity.rgb * L.specular.rgb, 1.0f);
        
        diffuse *= intensity;
        specular *= intensity;
        diffuse *= att;
        specular *= att;
    }
	
}

PixelOutput main(PixelInput input)
{   
    PixelOutput output;
    float4 wPos = g_posTex.Sample(g_sampler, input.tex);
    float4 normal = g_normalTex.Sample(g_sampler, input.tex);
    float reflection = g_reflectivityTex.Sample(g_sampler, input.tex);
    float4 albedo = g_abeldoTex.Sample(g_sampler, input.tex);
    float4 spec = g_specualrTex.Sample(g_sampler, input.tex);
    float3 viewDir = normalize(g_camPos.xyz - wPos.xyz);
    float occlusion = g_SSAOTex.Sample(g_sampler, input.tex);
    
    bool isCubeMap = length(normal) ? false : true;
    [branch]
    if (isCubeMap)
    {   
        output.mainColor = albedo;
        output.brightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);
        return output;
    }

    float4 lightVal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A, D, S;
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
    
    [unroll]
    for (int i = 0; i < g_dirCnt; ++i)
    {
        float3 lightDir = normalize(g_DirLight[i].position.xyz - wPos.xyz);
        CalDirectionalLight(g_DirLight[i], normal.xyz, viewDir, lightDir, spec, A, D, S);
        //shadowFactor = CalShadowFactor(g_shadowSampler, dirShadowMaps[i], dirShadowPos[i], normal, lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
        
    }
    [unroll]
    for (int j = 0; j < g_CubeCnt; ++j)
    {
        float3 lightDir = g_CubeLight[j].position - wPos.xyz;
        CalCubeLight(g_CubeLight[j], lightDir, normal.xyz, viewDir, spec, A, D, S);
        //shadowFactor = CalOmniDirShadowFactor(g_cubeShadowSampler, omniDirShadowMaps[j], -lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
    }
    [unroll]
    for (int k = 0; k < g_sptCnt; ++k)
    {   
        float3 lightDir = (g_SpotLight[k].position - wPos.xyz);
        CalSpotLight(g_SpotLight[k], lightDir, normal.xyz, viewDir, spec, A, D, S);
        lightDir = normalize(lightDir);
        shadowFactor = CalShadowFactor(g_shadowSampler, g_spotShadowMaps[k], spotShadowPos[k], normal.xyz, lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
    }

    wPos.z = 1.0f;
    viewDir = g_camPos - wPos;
    float3 r = reflect(viewDir, normal.xyz);
    float3 envColor = g_envMap.Sample(g_cubeShadowSampler, r);
    envColor *= reflection; //a=reflection;
    lightVal.xyz = (1.0 - reflection) * lightVal.xyz;
    output.mainColor.xyz = lightVal.xyz+envColor;
    output.mainColor.a = 1.0f;
    float brightness = dot(output.mainColor.rgb, float3(0.2126, 0.7152, 0.0722));
    [branch]
    if (brightness > 1.0f)
    {
        output.brightColor = float4(output.mainColor.rgb, 1.0f);
    }
    else
    {
        output.brightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    }
	return output;
}