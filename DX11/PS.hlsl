Texture2D shaderTexture;
Texture2D shadowMap;
SamplerState SampleType;
SamplerComparisonState shadowSampler;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
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

static const float SMAP_SIZE = 2048.0f;
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalShadowFactor(SamplerComparisonState shadowSampler,
                        Texture2D shadowMap,
                        float4 shadowPos)
{
    shadowPos.xyz /= shadowPos.w;
    shadowPos.x = shadowPos.x * 0.5f + 0.5f;
    shadowPos.y = shadowPos.y * -0.5f + 0.5f;
    float depth = shadowPos.z;
    
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
	float3 normal, float3 toEye, float4 wPos,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float3 lightDir = normalize(L.position-wPos.xyz);

    ambient = material.ambient * L.ambient;
    normal = normalize(normal);
    float diffuseFactor = dot(lightDir, normal);
    if (diffuseFactor > 0.0f)
    {
        float3 r = normalize(reflect(-lightDir, normal));
        float specFactor = pow(max(dot(normalize(toEye),r),0.0f), material.specular.w);
        
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        specular = specFactor * material.specular * L.specular;
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
    float4 texColor = shaderTexture.Sample(SampleType, input.tex);
    clip(texColor.a - 0.1f );
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float4 A, D, S;
    float shadowFactor = CalShadowFactor(shadowSampler, shadowMap, input.shadowPos);
    
    CalDirectionalLight(gMaterial, dirLight, input.normal, input.toEye, input.wPosition, A, D, S);
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
    
    return litColor;
}
