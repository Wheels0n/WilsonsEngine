Texture2D g_fullScreenDepth;
SamplerState g_WrapSampler;

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

float main(PixelInputType input) : SV_TARGET
{

    return g_fullScreenDepth.Sample(g_WrapSampler, input.tex);
}