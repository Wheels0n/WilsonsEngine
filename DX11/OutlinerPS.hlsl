struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D g_abeldoTex;
SamplerState g_sampler : register(s0);
float4 main(PixelInputType input) : SV_TARGET
{   
    float4 color = g_abeldoTex.Sample(g_sampler, input.tex);
    clip(color.a - 0.1f);
    return float4(1.0f, 0.34f, 0.0F, 1.0f);
}