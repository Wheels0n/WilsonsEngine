struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
Texture2D g_abeldoTex;
SamplerState g_sampler : register(s0);
float4  main(PixelInputType input) :SV_Target
{   
    float4 color = g_abeldoTex.Sample(g_sampler, input.tex);
    clip(color.a - 0.1f);
    
    float depth = input.position.z;
    return float4(depth, depth, depth, 1.0f);
}