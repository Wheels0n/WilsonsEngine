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
    output.position = float4(input.position, 1.0f);
    output.tex = input.tex;
	return output;
}