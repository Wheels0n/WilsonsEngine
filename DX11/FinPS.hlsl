Texture2D sceneTex;
SamplerState texSampler;

cbuffer Exporsure
{
    float exposure;
    float3 pad;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static const float GAMMA = 2.2f;

float4 main(PixelInputType input) : SV_Target
{   
    float3 hdrColor = sceneTex.Sample(texSampler, input.tex).rgb;
    
    hdrColor = float3(1.0f, 1.0f, 1.0f) - exp(-hdrColor * exposure);
    hdrColor = pow(hdrColor, float3(1.0f / GAMMA, 1.0f / GAMMA, 1.0f / GAMMA));
    return float4(hdrColor, 1.0f);
}