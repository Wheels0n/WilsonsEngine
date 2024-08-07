cbuffer MatrixBuffer
{
    matrix g_worldMat;
    matrix g_viewMat;
    matrix g_invWorldMat;
    matrix g_invWVMat;
    matrix g_wvpMat;
};
float4 main( float3 pos : POSITION ) : SV_POSITION
{   
    float4 output = float4(pos, 1.0f);
    output = mul(output, g_worldMat);
	return output;
}