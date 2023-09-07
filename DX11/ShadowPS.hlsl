struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
Texture2D abeldoTex;
SamplerState SampleType : register(s0);
void  main(PixelInputType input)
{   
    float4 color = abeldoTex.Sample(SampleType, input.tex);
    clip(color.a - 0.1f);
}