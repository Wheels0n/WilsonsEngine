struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
Texture2D g_abeldoTex;
SamplerState g_sampler : register(s0);
void  main(PixelInputType input)
{   
    float4 color = g_abeldoTex.Sample(g_sampler, input.tex);
    clip(color.a - 0.1f);
}