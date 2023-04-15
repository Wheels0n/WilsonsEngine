cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CamBuffer
{
	float4 m_camPos;
};

cbuffer PerModel
{
    bool isInstanced;
    bool hasSpecular;
    bool hasNormal;
    bool Padding;
};

struct VertexInputType
{
	float3 position : POSITION;
	float2 tex   : TEXTURE;
	float3 normal : NORMAL;
    matrix instanceMat : WORLD;
    uint InstanceID : SV_InstanceID;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex   : TEXTURE;
	float3 normal : NORMAL;
	float3 toEye: VIEW;
};

PixelInputType main(VertexInputType input)  
{
	PixelInputType output;

    float4 position = float4(input.position, 1.0f);
    if (isInstanced==true)
    {
        output.position = mul(position, input.instanceMat);

    }
    else
    {
        output.position = mul(position, worldMatrix);
    }
	
    output.toEye = output.position.xyz - m_camPos.xyz;
    output.toEye = normalize(output.toEye);

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
	return output;
}