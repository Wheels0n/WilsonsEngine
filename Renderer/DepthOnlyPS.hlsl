struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D g_abeldoTex;
SamplerState g_wrapSampler : register(s0);
float4 main(PixelInputType input) : SV_Target
{
    float4 color = g_abeldoTex.Sample(g_wrapSampler, input.tex);
    clip(color.a-0.1f);
    return float4(0.0f, 0.0f, 0.0f, 1.0f);
}