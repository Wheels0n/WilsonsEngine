Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex    : TEXTURE;
	float3 normal : NORMAL;
};
cbuffer Light
{
	float4 diffuse;
	float3 direction;
	float  padding;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 outputColor, lightColor;
    float  lightIntensity;
    
	outputColor = shaderTexture.Sample(SampleType, input.tex);
	lightColor  = diffuse;

	lightIntensity = dot(input.normal, -direction);
	if (lightIntensity > 0.0f)
	{
		lightColor *= lightIntensity;
	}


	outputColor = outputColor* lightColor;

	return outputColor;
}