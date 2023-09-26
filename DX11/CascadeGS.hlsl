struct GSOutput
{
	float4 pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

cbuffer cubeMap
{
    matrix g_lightViewMatrices[5];
};

[maxvertexcount(15)]
void main(
	triangle float4 input[3] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	for (uint i = 0; i < 5; i++)
	{	
        GSOutput element;
        element.RTIndex = i;
        for (int j = 0; j < 3;j++)
        {
            element.pos = mul(input[j], g_lightViewMatrices[i]);
            output.Append(element);
        }
        output.RestartStrip();
		
	}
}