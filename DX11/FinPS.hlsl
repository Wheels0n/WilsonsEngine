Texture2D g_sceneTex;
SamplerState g_sampler;

cbuffer Exporsure
{
    float g_exposure;
    float3 pad;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static const float _GAMMA = 2.2f;

float4 main(PixelInputType input) : SV_Target
{   
    float3 hdrColor = g_sceneTex.Sample(g_sampler, input.tex).rgb;
    
    hdrColor = float3(1.0f, 1.0f, 1.0f) - exp(-hdrColor * g_exposure);
    hdrColor = pow(hdrColor, float3(1.0f / _GAMMA, 1.0f / _GAMMA, 1.0f / _GAMMA));
    return float4(hdrColor, 1.0f);
}