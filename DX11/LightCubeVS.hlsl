struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightSpaceMat;
};

PixelInputType main(VertexInputType input) 
{   
    PixelInputType output;
    
    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    
    return output;
}