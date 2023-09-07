struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D abeldoTex;
SamplerState SampleType : register(s0);
float4 main(PixelInputType input) : SV_TARGET
{   
    float4 color = abeldoTex.Sample(SampleType, input.tex);
    clip(color.a - 0.1f);
    return float4(1.0f, 0.67451f, 0.10980, 1.0f);
}