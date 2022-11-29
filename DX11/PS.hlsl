Texture2D shaderTexture;
SamplerState SampleType;

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex    : TEXTURE;
};

float4 main(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
    textureColor = shaderTexture.Sample(SampleType, input.tex);

	return textureColor;
}