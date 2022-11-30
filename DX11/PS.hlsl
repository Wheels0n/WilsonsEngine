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
    float  lightAmount;
    
	outputColor = shaderTexture.Sample(SampleType, input.tex);
	lightAmount = dot(direction, input.normal);
	lightColor  = diffuse;
	lightColor *= lightAmount;
	outputColor = mul(outputColor, lightColor);
	

	return outputColor;
}