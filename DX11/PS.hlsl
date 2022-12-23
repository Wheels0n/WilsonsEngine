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
	float4 specularColor, texColor, diffuseColor, outputColor;
	float3 lightDir, reflection, viewDirection;
	float  lightIntensity;

	texColor = shaderTexture.Sample(SampleType, input.tex);
	lightDir = -direction;

	lightIntensity = saturate(dot(input.normal, lightDir));
	diffuseColor = diffuse * lightIntensity;
	diffuseColor = saturate(diffuseColor);

	specularColor = specular;
	if (lightIntensity > 0)
	{
		reflection = reflect(input.normal, lightDir);
		reflection = normalize(reflection);
		viewDirection = normalize(input.viewDir);
		specularColor = pow(saturate(dot(reflection, -viewDirection)), specPow);
		specularColor = saturate(specularColor);
		diffuseColor = diffuseColor * texColor;
	}

	outputColor = saturate(ambient + diffuseColor + specularColor);
	return outputColor;
	
}