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
    matrix g_wvpMat;
    matrix g_wvpLitMatrix;
};


PixelInputType main( VertexInputType input ) 
{   
    PixelInputType output;
    output.pos = mul(float4(input.pos, 0.0f),g_wvpMat);
    output.pos.z = output.pos.w;
    output.tex = input.pos;
    output.tex.x *= -1;
  
    return output;
}