struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D g_SSAOTex;
SamplerState g_sampler;
float4 main(PixelInputType input) : SV_TARGET
{
    float2 texelSize;
    g_SSAOTex.GetDimensions(texelSize.x, texelSize.y);
    texelSize = 1.0f / texelSize;
    float result = 0.0f;
    for (int x = -2; x < 2;++x)
    {
        for (int y = -2; y < 2;++y)
        {
            float2 offset = float2(x, y) * texelSize;
            result += g_SSAOTex.Sample(g_sampler, input.tex+offset);
        }

    }
    
    return result / (4.0f * 4.0f);

}