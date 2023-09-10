Texture2D g_tex;
SamplerState g_sampler;
cbuffer axis
{   
    bool g_bHorizontal;
    bool3 padding;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static float _WEIGHT[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

float4 main(PixelInputType input) : SV_TARGET
{   
    float sizeX, sizeY;
    g_tex.GetDimensions(sizeX, sizeY);
    float2 tex_offset = float2(1 / sizeX, 1 / sizeY);
    float3 result = g_tex.Sample(g_sampler, input.tex).rgb * _WEIGHT[0];
    
    [branch]
    if (g_bHorizontal)
    {   
        [unroll]
        for (int i = 1; i < 5;++i)
        {
            result += g_tex.Sample(g_sampler, input.tex + float2(tex_offset.x * i, 0.0)).rgb * _WEIGHT[i];
            result += g_tex.Sample(g_sampler, input.tex - float2(tex_offset.x * i, 0.0)).rgb * _WEIGHT[i];
        }

    }
    else
    {   
        [unroll]
        for (int i = 1; i < 5; ++i)
        {
            result += g_tex.Sample(g_sampler, input.tex + float2(0.0f, tex_offset.y * i)).rgb * _WEIGHT[i];
            result += g_tex.Sample(g_sampler, input.tex - float2(0.0f, tex_offset.y * i)).rgb * _WEIGHT[i];
        }
    }
    return float4(result, 1.0f);
}