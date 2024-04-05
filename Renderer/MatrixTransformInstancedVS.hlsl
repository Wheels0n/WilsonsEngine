struct VertexInputType
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
    uint instanceId : SV_InstanceID;
};
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};
struct MatrixBuffer
{
    matrix g_worldMat;
    matrix g_viewMat;
    matrix g_invWorldMat;
    matrix g_invWVMat;
    matrix g_wvpMat;
};
cbuffer instanceMatrix
{
    MatrixBuffer g_instance[10];
};

PixelInputType main(VertexInputType input)
{
    PixelInputType output;
    
    output.position = float4(input.position, 1.0f);
    output.position = mul(output.position, g_instance[input.instanceId].g_wvpMat);
    output.tex = input.tex;
    
    return output;
}