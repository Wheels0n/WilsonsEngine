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
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightSpaceMat;
};

GS_INPUT main(VertexInputType input)
{   
    GS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), worldMatrix);
    output.tex = input.tex;
	return output;
}