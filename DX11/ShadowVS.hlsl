cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightSpaceMat;
};

struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    row_major float4x4 instanceMat : WORLD;
    uint InstanceID : SV_InstanceID;
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
    
    output.position = mul(pos, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
    output.tex = input.tex;
    
	return output;
}