TextureCube SkyBox;
SamplerState Sampler;

struct PixelInputType
{
    float4 pos : SV_POSITION;
    float3 tex : TEXTURE;
};

float4 main(PixelInputType input) : SV_TARGET
{
    return SkyBox.Sample(Sampler, input.tex);
}