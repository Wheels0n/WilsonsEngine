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
    output.position = mul(output.position, lightSpaceMat);
    
    output.tex = input.tex;
    
	return output;
}