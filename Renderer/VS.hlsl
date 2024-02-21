
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
    bool hasNormal;
    bool hasSpecular;
    bool hasAlpha;
};

cbuffer DirLightMatrices
{
    matrix dirLightMatrices[10];
    uint dirLightCnt;
};

struct VertexInputType
{
	float3 position : POSITION;
    float2 tex      : TEXTURE;
	float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    row_major float4x4 instanceMat : WORLD;
    uint InstanceID : SV_InstanceID;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex     : TEXTURE;
    float3 normal  : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float3 viewDir  : VIEW;
    float4 shadowPos[10] : POSITION0;
    float4 wPosition : POSITION10;
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
    
    for (int i = 0; i < 10; ++i)
    {
        output.shadowPos[i] = output.wPosition;
    }
    
    for (int j = 0; j < dirLightCnt;++j)
    {
        output.shadowPos[j] = mul(output.shadowPos[j], dirLightMatrices[j]);
    }
    
    output.viewDir = m_camPos.xyz - output.position.xyz;
    output.viewDir = normalize(output.viewDir);

	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.tex = input.tex;
	
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);
    
    [branch]
    if(hasNormal==true)
    {
        output.tangent = mul(input.tangent, (float3x3) worldMatrix);
        output.tangent = normalize(output.tangent);
        output.binormal = cross(output.normal, output.tangent);
        output.binormal = normalize(output.binormal);
    }
    else
    {   
        output.binormal = float3(0.0f, 0.0f, 0.0f);
        output.tangent = input.tangent;
    }
 
    
	return output;
}