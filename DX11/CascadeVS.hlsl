cbuffer MatrixBuffer
{
    matrix g_worldMatrix;
    matrix g_viewMatrix;
    matrix g_projectionMatrix;
    matrix g_extraMat;
};
float4 main( float3 pos : POSITION ) : SV_POSITION
{   
    float4 output = float4(pos, 1.0f);
    output = mul(output, g_worldMatrix);
	return output;
}