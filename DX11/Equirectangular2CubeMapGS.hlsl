struct PixelInputType
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};


cbuffer cubeMap
{
    matrix lightViewMatrices[6];
};

[maxvertexcount(18)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream<PixelInputType> cubeMapStream
)
{
    PixelInputType output;
    for (int face = 0; face < 6; face++)
    {
 
        output.RTIndex = face;
        for (int v = 0; v < 3; v++)
        {
            output.wPos = input[v];
            output.pos = mul(input[v], lightViewMatrices[face]);
            cubeMapStream.Append(output);
        }
        cubeMapStream.RestartStrip(); 
        
    }
}