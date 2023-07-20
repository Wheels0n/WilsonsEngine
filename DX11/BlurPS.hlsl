Texture2D tex;
SamplerState texSampler;
cbuffer axis
{   
    bool isHorizontal;
    bool3 padding;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static float weight[5] = { 0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162 };

float4 main(PixelInputType input) : SV_TARGET
{   
    float sizeX, sizeY;
    tex.GetDimensions(sizeX, sizeY);
    float2 tex_offset = float2(1 / sizeX, 1 / sizeY);
    float3 result = tex.Sample(texSampler, input.tex).rgb * weight[0];
    
    [branch]
    if(isHorizontal)
    {   
        [unroll]
        for (int i = 1; i < 5;++i)
        {
            result += tex.Sample(texSampler, input.tex + float2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += tex.Sample(texSampler, input.tex - float2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }

    }
    else
    {   
        [unroll]
        for (int i = 1; i < 5; ++i)
        {
            result += tex.Sample(texSampler, input.tex + float2(0.0f, tex_offset.y * i)).rgb * weight[i];
            result += tex.Sample(texSampler, input.tex - float2(0.0f, tex_offset.y * i)).rgb * weight[i];
        }
    }
    return float4(result, 1.0f);
}