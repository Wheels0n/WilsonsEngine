struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
struct PixelOutput
{
    float4 SSAO : SV_Target0;
    float4 Debug : SV_Target1;
};
Texture2D g_SSAOTex;
SamplerState g_WrapSampler;
PixelOutput main(PixelInputType input)
{   
    PixelOutput output;
    float2 texelSize;
    g_SSAOTex.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0f / texelSize;
    float result = 0.0f;
    for (int x = -2; x < 2;++x)
    {
        for (int y = -2; y < 2;++y)
        {
            float2 offset = float2(x, y) * texelSize;
            result += g_SSAOTex.Sample(g_WrapSampler, input.tex + offset);
        }

    }
    
    result /= (4.0f * 4.0f);
    output.SSAO = result;
    output.Debug = float4(result, result, result, 1.0f);
    return output;
}