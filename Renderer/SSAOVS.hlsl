struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float2 viewRay :ViewRay;
};

cbuffer frustumInfo
{
    float g_ratio;
    float g_TanHalfFov;
    float2 padding;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    output.position = float4(input.position, 1.0f);
    output.tex = input.tex;
    output.viewRay.x = output.position.x * g_ratio * g_TanHalfFov;
    output.viewRay.y = output.position.y * g_TanHalfFov;
    return output;
}