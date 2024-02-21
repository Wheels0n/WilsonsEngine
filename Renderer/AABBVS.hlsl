cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix g_extraMat;
};

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    float4 output = float4(pos,1.0f);
    output = mul(output, worldMatrix);
    output = mul(output, viewMatrix);
    output = mul(output, projectionMatrix);
    return output;
}