struct VertexInputType
{
    float3 pos : POSITION;
    float2 tex : TEXTURE;
};

struct GS_INPUT
{
    float4 pos : SV_Position;
    float2 tex : TEXTURE;
};

cbuffer MatrixBuffer
{
    matrix g_worldMatrix;
    matrix g_viewMatrix;
    matrix g_projectionMatrix;
    matrix g_extraMat;
};

GS_INPUT main(VertexInputType input)
{   
    GS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), g_worldMatrix);
    output.tex = input.tex;
	return output;
}