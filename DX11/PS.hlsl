Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex    : TEXTURE;
	float3 normal : NORMAL;
    float3 toEye : VIEW;
};
struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float3 direction;
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


void CalDirectionalLight(Material material, DirectionalLight L,
	float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 specular)
{
    ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

    float3 lightDir = -L.direction;
	
    ambient = material.ambient * L.ambient;

    float diffuseFactor = dot(lightDir, normal);
    if (diffuseFactor > 0.0f)
    {
        float3 r = reflect(-lightDir, normal);
        float specFacotr = pow(max(dot(r, toEye), 0.0f), material.specular.w);
        
        diffuse = diffuseFactor * material.diffuse * L.diffuse;
        specular = specFacotr * material.specular * L.specular;
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
    
    float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float4 specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    float4 A, D, S;
    
    CalDirectionalLight(gMaterial, dirLight, input.normal, input.toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;

    CalPointLight(gMaterial, pointLight, (float3) input.position, input.normal, input.toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    

    CalSpotLight(gMaterial, spotLight, (float3) input.position, input.normal, input.toEye, A, D, S);
    ambient += A;
    diffuse += D;
    specular += S;
    
    float4 litColor = texColor * (ambient + diffuse) + specular;

    litColor.a = texColor.a * gMaterial.diffuse.a;
    
    return litColor;
}
