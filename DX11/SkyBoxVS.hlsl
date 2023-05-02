struct VertexInputType
{
    float3 pos : POSITION;
};

struct PixelInputType
{
    float4 pos : SV_POSITION;
    float3 tex : TEXTURE;

};

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

PixelInputType main( VertexInputType input ) 
{   
    PixelInputType output;
    output.pos = mul(float4(input.pos, 0.0f),viewMatrix);
    output.pos = mul(output.pos, projectionMatrix);
    output.pos.z = output.pos.w;
    output.tex = input.pos;
    output.tex.x *= -1;
  
    return output;
}