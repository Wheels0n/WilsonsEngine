struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 wPosition : POSITION0;
    float4 vPosition : POSITION1;
    float3 vNormal : vNORMAL;
};

cbuffer MatrixBuffer
{
    matrix g_worldMatrix;
    matrix g_viewMatrix;
    matrix g_projectionMatrix;
    matrix g_lightSpaceMat;
};

cbuffer PerModel
{
    bool g_isInstanced;
    bool g_hasNormal;
    bool g_hasSpecular;
    bool g_hasEmssive;
    bool g_hasAlpha;
    float3 pad;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, g_worldMatrix);
    output.wPosition = output.position;
   
    output.position = mul(output.position, g_viewMatrix);
    output.vPosition = output.position;
    output.position = mul(output.position, g_projectionMatrix);
    
    output.tex = input.tex;
    
    output.normal = mul(input.normal, (float3x3) g_worldMatrix);
    output.normal = normalize(output.normal);
    output.vNormal = mul(output.normal, (float3x3) g_viewMatrix);
    output.vNormal = normalize(output.vNormal);
    
    output.tangent = float3(0.0f, 0.0f, 0.0f);
    output.binormal = output.tangent;
    [branch]
    if (g_hasNormal)
    {
    
        output.tangent = mul(input.tangent, (float3x3) g_worldMatrix);
        output.tangent = normalize(output.tangent);
        output.binormal = cross(output.normal, output.tangent);
        output.binormal = normalize(output.binormal);
    }
    return output;
}