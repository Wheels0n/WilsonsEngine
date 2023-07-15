struct PS_INPUT
{
	float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};
struct GS_INPUT
{
    float4 pos : SV_Position;
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
            cubeMapStream.Append(output);
        }
        cubeMapStream.RestartStrip();//�⺻�� Strip�̶� List�� �Ϸ��� �ﰢ������ ����
        
    }
}