Texture2D diffuseMap;
Texture2D shadowMap;
Texture2D specularMap;
Texture2D normalMap;
Texture2D alphaMap;
SamplerState SampleType;
SamplerComparisonState shadowSampler;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 toEye : VIEW;
    float4 shadowPos : POSITION1;
    float4 wPosition : POSITION2;
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
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};

cbuffer gLight
{   
	DirectionalLight  dirLight;
	PointLight        pointLight;
	SpotLight		  spotLight;
};
cbuffer Material
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

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalShadowFactor(SamplerComparisonState shadowSampler,
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
void CalDirectionalLight(Material material, DirectionalLight L,
	float3 normal, float3 toEye, float3 lightDir, float4 specularIntensity,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    ambient = material.ambient * L.ambient;
   
    float diffuseFactor = max(dot(lightDir, normal), 0.0f);
    diffuse = diffuseFactor * material.diffuse * L.diffuse;
    [branch]
    if(diffuseFactor!=0.0f)
    {
        float3 h = normalize(toEye + lightDir);
        float specFactor = pow(max(dot(normal, h), 0.0f), material.specular.w);
        specular = specFactor * specularIntensity * material.specular * L.specular;
    }
 
}
void CalPointLight(Material material, PointLight L, float3 pos, float3 normal, float3 toEye,
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
        float3 v = reflect(-lightVec, normal);
        float specFactor = pow(max(dot(v, toEye), 0.0f), material.specular.w);
		
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        specular = specFactor * material.specular * L.specular;
    }
	
    float att = 1.0f / dot(L.att, float3(1.0f, distance, distance * distance));
	
    diffuse *= att;
    specular *= att;
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

float4 main(PixelInputType input) : SV_TARGET
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
        
    
    float3 lightDir = normalize(dirLight.position - input.wPosition.xyz);
    float3 normal = normalize(input.normal);
    [branch]
    if (hasNormal==true)
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
    float shadowFactor = CalShadowFactor(shadowSampler, shadowMap, input.shadowPos, normal, lightDir);
  
    CalDirectionalLight(gMaterial, dirLight, normal, input.toEye, lightDir, specularIntensity, A, D, S);
    ambient += A;
    diffuse += D * shadowFactor;
    specular += S * shadowFactor;

    /*CalPointLight(gMaterial, pointLight, (float3) input.position, input.normal, input.toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    

    CalSpotLight(gMaterial, spotLight, (float3) input.position, input.normal, input.toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    */


    float4 litColor = texColor * (ambient + diffuse + specular);

    litColor.a = texColor.a * gMaterial.diffuse.a;
    litColor = litColor * alphaIntensity;
   
    return litColor;
}
