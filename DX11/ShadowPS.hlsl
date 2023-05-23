struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

float4 main() : SV_TARGET
{
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}