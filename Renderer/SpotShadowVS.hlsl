cbuffer MatrixBuffer
{
    matrix g_worldMat;
    matrix g_viewMat;
    matrix g_invWorldMat;
    matrix g_invWVMat;
    matrix g_wvpMat;
};

cbuffer SpotLightMatric
{
    matrix g_vpLitMatrix;
};

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

PixelInputType main(VertexInputType input)
{   
    PixelInputType output;
    
    float4 pos = float4(input.position, 1.0f);
    output.position = mul(pos, g_worldMat);
    output.position = mul(output.position, g_vpLitMatrix);
    output.tex = input.tex;
    
	return output;
}