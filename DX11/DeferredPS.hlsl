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
    float spot;

    float3 att;
    float pad;
};

Texture2D posTex;
Texture2D normalTex;
Texture2D abeldoTex;
Texture2D specualrTex;
Texture2D dirShadowMaps[10];
TextureCube omniDirShadowMaps[45];
SamplerState SampleType : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_dirShadowSampler : register(s2);

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
cbuffer CamBuffer
{
    float4 g_camPos;
};

static const float SMAP_SIZE = 1024.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;
static const float FAR_PLANE = 25.0f;

float CalDirShadowFactor(SamplerComparisonState shadowSampler,
                        Texture2D shadowMap,
                        float4 shadowPos,
                        float3 normal, float3 lightDir)
{
    shadowPos.xyz /= shadowPos.w;
    shadowPos.x = shadowPos.x * 0.5f + 0.5f;
    shadowPos.y = shadowPos.y * -0.5f + 0.5f;
    float bias = max((0.05f * (1.0f - dot(normal, lightDir))), 0.0005f);
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
void CalPointLight(float4 albedo, PointLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	
    float distance = length(lightDir);
	
    [branch]
    if (distance > L.range)
    {
        return;
    }
	
    lightDir = normalize(lightDir);
	
    ambient = float4(albedo.rgb, 1.0f) * L.ambient;
	
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
	
    if (diffuseFactor != 0.0f)
    {
        diffuse = diffuseFactor * float4(albedo.rgb, 1.0f) * L.diffuse;
        
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), specularIntensity.w);
        specular = float4(specFactor * specularIntensity.rgb * L.specular.rgb, 1.0f);
	    
        float att = 1.0f / dot(L.att, float3(1.0f, distance, distance * distance));
        diffuse *= att;
        specular *= att;
    }
	
    
}
void CalSpotLight(float4 albedo, SpotLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
    float distance = length(lightDir);
	
    if (distance > L.range)
    {
        return;
    }
	
    lightDir = normalize(lightDir);
	
    ambient = float4(albedo.rgb, 1.0f) * L.ambient;
	
    float diffuseFactor = dot(lightDir, normal);
	
    if (diffuseFactor > 0.0f)
    {
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), specularIntensity.w);
		
        diffuse = diffuseFactor * float4(albedo.rgb, 1.0f) * L.diffuse;
        specular = float4(specFactor * specularIntensity.rgb * L.specular.rgb, 1.0f);
    }
	
    float spot = pow(max(dot(lightDir, L.direction), 0.0f), L.spot);
	
    float att = spot / dot(L.att, float3(1.0f, distance, distance * distance));
	
    ambient *= spot;
    diffuse *= att;
    specular *= att;
}

PixelOutput main(PixelInput input)
{   
    PixelOutput output;
    float4 wPos = posTex.Sample(SampleType, input.tex);
    float3 normal = normalTex.Sample(SampleType, input.tex);
    float4 albedo = abeldoTex.Sample(SampleType, input.tex);
    float4 spec = specualrTex.Sample(SampleType, input.tex);
    float3 viewDir = normalize(g_camPos.xyz - wPos.xyz);
    
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
    [Flatten]
    for (int i = 0; i < dirCnt; ++i)
    {
        dirShadowPos[i] = mul(wPos, dirLightMatrices[i]);
    }
    
    [flatten]
    for (int i = 0; i < dirCnt; ++i)
    {
        float3 lightDir = normalize(cbDirLight[i].position.xyz - wPos.xyz);
        CalDirectionalLight(cbDirLight[i], normal, viewDir, lightDir, spec, A, D, S);
        shadowFactor = CalDirShadowFactor(g_dirShadowSampler, dirShadowMaps[i], dirShadowPos[i], normal, lightDir);
        lightVal += (A + shadowFactor * (D + S))*albedo;
        
    }
    [flatten]
    for (int j = 0; j < pntCnt; ++j)
    {
        float3 lightDir = cbPointLight[j].position - wPos.xyz;
        CalPointLight(albedo, cbPointLight[j], lightDir, normal, viewDir, spec, A, D, S);
        shadowFactor = CalOmniDirShadowFactor(g_cubeShadowSampler, omniDirShadowMaps[j], -lightDir);
        lightVal += (A + shadowFactor * (D + S)) * albedo;
    }
    [flatten]
    for (int k = 0; k < sptCnt; ++k)
    {   
        float3 lightDir = (cbSpotLight[i].position - wPos.xyz);
        CalSpotLight(albedo, cbSpotLight[k], lightDir, normal, viewDir, spec, A, D, S);
        lightVal += (A + shadowFactor * (D + S)) * albedo;
    }

    output.mainColor = lightVal;
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