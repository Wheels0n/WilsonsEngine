Texture2D diffuseMap;
Texture2D specularMap;
Texture2D normalMap;
Texture2D alphaMap;
Texture2D dirShadowMaps[1];
TextureCube CubeShadowMaps[1];
SamplerState SampleType : register(s0);
SamplerState g_cubeShadowSampler : register(s1);
SamplerComparisonState g_dirShadowSampler : register(s2);

struct PixelOuput
{
    float4 mainColor : SV_Target0;
    float4 brightColor : SV_Target1;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDir : VIEW;
    float4 shadowPos[10] : POSITION0;
    float4 wPosition : POSITION10;
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
    float spot;

    float3 att;
    float pad;
};
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};

cbuffer cbLight
{   
    DirectionalLight cbDirLight[10];
    uint dirCnt;
    CubeLight cbCubeLight[48];
    uint CubeCnt;
    SpotLight cbSpotLight[20];
    uint sptCnt;
    uint padding;
};
cbuffer cbMaterial
{
    Material gMaterial;
};
cbuffer PerModel
{
    bool isInstanced;
    bool hasNormal;
    bool hasSpecular;
    bool hasAlpha;
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
    float depth = shadowPos.z-bias;
    
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
float CalCubeShadowFactor(SamplerState shadowSampler,
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
    for (int i = 0; i < 20;++i)
    {   
        float sampledDepth = shadowMap.SampleLevel(shadowSampler, fragToLight + offesets[i],0).r;
        sampledDepth *= FAR_PLANE;
        if(sampledDepth>curDepth)
        {
            percentLit += 1.0f;
        }
        
    }
    return percentLit /= 20.0f;

}
void CalDirectionalLight(Material material, DirectionalLight L,
	float3 normal, float3 viewDir, float3 lightDir, float4 specularIntensity,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ambient = material.ambient * L.ambient;
   
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
    [branch]
    if(diffuseFactor!=0.0f)
    {
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        
        float3 h = normalize(viewDir + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), material.specular.w);
        specular = specFactor * specularIntensity * material.specular * L.specular;
    }
 
}
void CalCubeLight(Material material, CubeLight L, float3 lightDir, float3 normal, float3 toEye, float4 specularIntensity,
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
	
    ambient = material.ambient * L.ambient;
	
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
	
    if (diffuseFactor != 0.0f)
    {
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), material.specular.w);
        specular = specFactor * specularIntensity * material.specular * L.specular;
	    
        float att = 1.0f / dot(L.att, float3(1.0f, distance, distance * distance));
        diffuse *= att;
        specular *= att;
    }
	
    
}
void CalSpotLight(Material material, SpotLight L, float3 pos, float3 normal, float3 toEye,
out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
    float3 lightVec = L.position - pos;
	
    float distance = length(lightVec);
	
    if (distance > L.range)
    {
        return;
    }
	
    lightVec /= distance;
	
    ambient = material.ambient * L.ambient;
	
    float diffuseFactor = dot(lightVec, normal);
	
    if (diffuseFactor > 0.0f)
    {
        float3 r = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(r, toEye), 0.0f), material.specular.w);
		
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        specular = specFactor * material.specular * L.specular;
    }
	
    float spot = pow(max(dot(-lightVec, L.direction), 0.0f), L.spot);
	
    float att = spot / dot(L.att, float3(1.0f, distance, distance * distance));
	
    ambient *= spot;
    diffuse *= att;
    specular *= att;
}

PixelOuput main(PixelInputType input)
{   

    float4 texColor = diffuseMap.Sample(SampleType, input.tex);
    clip(texColor.a - 0.1f );
    
    
    float4 alphaIntensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
    [branch]
    if (hasAlpha)
    {
        alphaIntensity = alphaMap.Sample(SampleType, input.tex);
        clip(dot((float3) alphaIntensity, (float3)alphaIntensity) - 0.1f);
    }
    
    float4 specularIntensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
    [branch]
    if (hasSpecular == true)
    {
        specularIntensity=specularMap.Sample(SampleType, input.tex);
    }
        
    float3 normal = normalize(input.normal);
    [branch]
    if (hasNormal)
    {
        float3 tangent = normalize(input.tangent);
        float3 binormal = normalize(input.binormal);
        float3x3 TBN = float3x3(tangent, binormal,normal);
        normal = normalMap.Sample(SampleType, input.tex);
        normal = normal * 2.0 - 1.0;
        normal = mul(normal, TBN);
        normal = normalize(normal);
    }
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 A, D, S;
    float shadowFactor=1.0f;
    input.viewDir = normalize(input.viewDir);
    for (int i = 0; i < dirCnt; ++i)
    {   
        float3 lightDir = normalize(cbDirLight[i].position - input.wPosition.xyz);
        CalDirectionalLight(gMaterial, cbDirLight[i], normal, input.viewDir, lightDir, specularIntensity, A, D, S);
        shadowFactor = CalDirShadowFactor(g_dirShadowSampler, dirShadowMaps[i], input.shadowPos[i], normal, lightDir);
        ambient += A;
        diffuse += D * shadowFactor;
        specular += S * shadowFactor;
    }
    for (int j = 0; j < CubeCnt; ++j)
    {   
        float3 lightDir = cbCubeLight[j].position - input.wPosition.xyz;
        CalCubeLight(gMaterial, cbCubeLight[j], lightDir, normal, input.viewDir, specularIntensity, A, D, S);
        shadowFactor = CalCubeShadowFactor(g_cubeShadowSampler, CubeShadowMaps[j], -lightDir);
       ambient += A;
       diffuse += D * shadowFactor;
       specular += S * shadowFactor;
    }
    
    for (int k = 0; k < sptCnt; ++k)
    {
        CalSpotLight(gMaterial, cbSpotLight[k], (float3) input.position, input.normal, input.viewDir, A, D, S);
        ambient += A;
        diffuse += D * shadowFactor;
        specular += S * shadowFactor;
    }

    PixelOuput output;
    output.mainColor = texColor * (ambient + diffuse) + specular;

    output.mainColor.a = texColor.a * gMaterial.diffuse.a;
    output.mainColor = output.mainColor * alphaIntensity;
    float brightness = dot(output.mainColor.rgb, float3(0.2126, 0.7152, 0.0722));
    [branch]
    if(brightness>1.0f)
    {
        output.brightColor = float4(output.mainColor.rgb, 1.0f);
    }
    else
    {
        output.brightColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    }
    
    return output;
}
