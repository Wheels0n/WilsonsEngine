struct PS_INPUT
{
	float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    float2 tex : TEXTURE;
    uint RTIndex : SV_RenderTargetArrayIndex;
};
struct GS_INPUT
{
    float4 pos : SV_Position;
    float2 tex : TEXTURE;
};

cbuffer cubeMap
{
    matrix lightViewMatrices[6];
};

[maxvertexcount(18)]
void main(
	triangle GS_INPUT input[3], 
	inout TriangleStream<PS_INPUT> cubeMapStream
)
{	
    PS_INPUT output;
    for (int face = 0; face < 6; face++)
    {
 
        output.RTIndex = face;
        for (int v = 0; v < 3; v++)
        {   
            output.wPos = input[v].pos;
            output.pos = mul(input[v].pos, lightViewMatrices[face]);
            output.tex = input[v].tex;
            cubeMapStream.Append(output);
        }
        cubeMapStream.RestartStrip();//기본이 Strip이라 List로 하려면 삼각형마다 갱신
        
    }
}