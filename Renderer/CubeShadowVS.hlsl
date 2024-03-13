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
    matrix g_worldMat;
    matrix g_viewMat;
    matrix g_invWorldMat;
    matrix g_invWVMat;
    matrix g_wvpMat;
};

GS_INPUT main(VertexInputType input)
{   
    GS_INPUT output;
    output.pos = mul(float4(input.pos, 1.0f), g_worldMat);
    output.tex = input.tex;
	return output;
}