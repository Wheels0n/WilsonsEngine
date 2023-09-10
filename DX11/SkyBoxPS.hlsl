TextureCube g_SkyBox;
SamplerState g_sampler;

struct PixelInputType
{
    float4 pos : SV_POSITION;
    float3 tex : TEXTURE;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return g_SkyBox.Sample(g_sampler, input.tex);
}