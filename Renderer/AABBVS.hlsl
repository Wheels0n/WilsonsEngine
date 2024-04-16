cbuffer MatrixBuffer
{
    matrix g_wvpMat;
};

float4 main( float3 pos : POSITION ) : SV_POSITION
{
    float4 output = float4(pos,1.0f);
    output = mul(output, g_wvpMat);
    return output;
}