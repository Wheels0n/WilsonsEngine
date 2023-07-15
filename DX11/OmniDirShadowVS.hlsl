struct VertexInputType
{
    float3 pos : POSITION;
};

struct GS_INPUT
{
    float4 pos : SV_Position;
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
	return output;
}