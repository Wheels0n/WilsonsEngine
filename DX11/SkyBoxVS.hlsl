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
    matrix g_worldMatrix;
    matrix g_viewMatrix;
    matrix g_projectionMatrix;
};

PixelInputType main( VertexInputType input ) 
{   
    PixelInputType output;
    output.pos = mul(float4(input.pos, 0.0f),g_viewMatrix);
    output.pos = mul(output.pos, g_projectionMatrix);
    output.pos.z = output.pos.w;
    output.tex = input.pos;
    output.tex.x *= -1;
  
    return output;
}