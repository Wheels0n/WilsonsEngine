struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

cbuffer MatrixBuffer
{
    matrix g_worldMatrix;
    matrix g_viewMatrix;
    matrix g_projectionMatrix;
    matrix g_extraMat;
};

PixelInputType main(VertexInputType input) 
{   
    PixelInputType output;
    
    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, g_worldMatrix);
    output.position = mul(output.position, g_viewMatrix);
    output.position = mul(output.position, g_projectionMatrix);
    
    output.tex = input.tex;
    
    return output;
}