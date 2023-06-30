cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
    matrix lightSpaceMat;
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
    float2 tex : TEXTURE;
	float3 normal : NORMAL;
    row_major float4x4 instanceMat : WORLD;
    uint InstanceID : SV_InstanceID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 toEye : VIEW;
    float4 shadowPos : POSITION1;
    float4 wPosition : POSITION2;
};

PixelInputType main(VertexInputType input)  
{
	PixelInputType output;

    float4 position = float4(input.position, 1.0f);
    [branch]
    if (isInstanced==true)
    {
        output.position = mul(position,
        worldMatrix + input.instanceMat);

    }
    else
    {
        output.position = mul(position, worldMatrix);
    }
    output.wPosition = output.position;
    
    output.shadowPos = output.position;
    output.shadowPos = mul(output.shadowPos, lightSpaceMat);
    
    output.toEye = m_camPos.xyz-output.position.xyz;
    output.toEye = normalize(output.toEye);

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
    
    
	return output;
}