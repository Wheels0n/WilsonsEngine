Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex    : TEXTURE;
	float3 normal : NORMAL;
	float3 viewDir : VIEW;
};
cbuffer Light
{   
	float4 specular;
	float4 ambient;
	float4 diffuse;
	float3 direction;
	float  specPow;
};


float4 main(PixelInputType input) : SV_TARGET
{
	float4 texColor, diffuseColor, view, specularColor;
    float3 lightDir, reflectV;
	float  diffuseIntensity;
    
	texColor = shaderTexture.Sample(SampleType, input.tex);

	lightDir = -direction;
	reflectV = normalize(reflect(lightDir, input.normal));

	diffuseIntensity = dot(input.normal, lightDir);
	diffuseColor = diffuse * diffuseIntensity;
	diffuseColor *= texColor;
	
	specularColor = specular;
	if (diffuseColor.x > 0.0f)
	{
		specularColor = pow(saturate(dot(reflectV, input.viewDir)), specPow);
	}
	

	return diffuseColor + ambient + specularColor;
}