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
Texture2D specualrTex;
Texture2D reflectivityTex;
Texture2D SSAOTex;
//Texture2D dirShadowMaps[1];
//TextureCube omniDirShadowMaps[1];
Texture2D spotShadowMaps[1];
TextureCube EnvMap;
SamplerState SampleType : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_shadowSampler : register(s2);

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
cbuffer CamBuffer
{
    float4 g_camPos;
};

static const float SMAP_SIZE = 1024.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;
static const float FAR_PLANE = 25.0f;

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
   
    return percentLit /= 9.0f;
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
        float sampledDepth = shadowMap.SampleLevel(shadowSampler, fragToLight + offesets[i],0).r;
        sampledDepth *= FAR_PLANE;
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
void CalPointLight(PointLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
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
    float4 wPos = posTex.Sample(SampleType, input.tex);
    float4 normal = normalTex.Sample(SampleType, input.tex);
    float reflection = reflectivityTex.Sample(SampleType, input.tex);
    float4 albedo = abeldoTex.Sample(SampleType, input.tex);
    float4 spec = specualrTex.Sample(SampleType, input.tex);
    float3 viewDir = normalize(g_camPos.xyz - wPos.xyz);
    float occlusion = SSAOTex.Sample(SampleType, input.tex);
    
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
    for (int i = 0; i < dirCnt; ++i)
    {
        dirShadowPos[i] = mul(wPos, dirLightMatrices[i]);
    }
    [unroll]
    for (int i = 0; i < sptCnt; ++i)
    {
        spotShadowPos[i] = mul(wPos, spotLightMatrices[i]);
    }
    
    [unroll]
    for (int i = 0; i < dirCnt; ++i)
    {
        float3 lightDir = normalize(cbDirLight[i].position.xyz - wPos.xyz);
        CalDirectionalLight(cbDirLight[i], normal, viewDir, lightDir, spec, A, D, S);
        //shadowFactor = CalShadowFactor(g_shadowSampler, dirShadowMaps[i], dirShadowPos[i], normal, lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
        
    }
    [unroll]
    for (int j = 0; j < pntCnt; ++j)
    {
        float3 lightDir = cbPointLight[j].position - wPos.xyz;
        CalPointLight(cbPointLight[j], lightDir, normal, viewDir, spec, A, D, S);
        //shadowFactor = CalOmniDirShadowFactor(g_cubeShadowSampler, omniDirShadowMaps[j], -lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
    }
    [unroll]
    for (int k = 0; k < sptCnt; ++k)
    {   
        float3 lightDir = (cbSpotLight[k].position - wPos.xyz);
        CalSpotLight(cbSpotLight[k], lightDir, normal, viewDir, spec, A, D, S);
        lightDir = normalize(lightDir);
        shadowFactor = CalShadowFactor(g_shadowSampler, spotShadowMaps[k], spotShadowPos[k], normal, lightDir);
        lightVal += (occlusion * A + shadowFactor * (D + S)) * albedo;
    }

    wPos.z = 1.0f;
    viewDir = g_camPos - wPos;
    float3 r = reflect(viewDir, normal);
    float3 envColor = EnvMap.Sample(g_cubeShadowSampler, r);
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