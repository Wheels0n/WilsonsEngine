
#define NOT_NORMAL 0

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
};

cbuffer MatrixBuffer
{
    matrix g_worldMatrix;
    matrix g_wvpMatrix;
    matrix g_invWorldMat;
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    output.position = float4(input.position, 1.0f);
    output.wPosition = mul(output.position, g_worldMatrix);
    output.position = mul(output.position, g_wvpMatrix);
    
    output.tex = input.tex;
    
    output.normal = mul(input.normal, (float3x3) transpose(g_invWorldMat));
    output.normal = normalize(output.normal);
#ifdef HAS_NORMAL 
   output.tangent = mul(input.tangent, (float3x3) transpose(g_invWorldMat));
   output.tangent = normalize(output.tangent);
   output.binormal = cross(output.normal, output.tangent);
   output.binormal = normalize(output.binormal);
#elif NOT_NORMAL  
    output.tangent = float3(0.0f, 0.0f, 0.0f);
    output.binormal = output.tangent;
#endif
    return output;
}