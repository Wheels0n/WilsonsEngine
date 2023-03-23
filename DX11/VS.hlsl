cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer camBuffer
{
	float4 m_camPos;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex   : TEXTURE;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex   : TEXTURE;
	float3 normal : NORMAL;
	float3 viewDir: VIEW;
};

PixelInputType main(VertexInputType input)  
{
	PixelInputType output;

	input.position.w = 1.0f;

	output.position = mul(input.position, worldMatrix);
	
	output.viewDir = output.position.xyz - m_camPos.xyz;
	output.viewDir = normalize(output.viewDir);

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	return output;
}