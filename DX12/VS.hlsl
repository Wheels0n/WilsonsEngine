cbuffer cbPerObject : register(b0)
{
	float4x4 gWorldViewProj;
}

void main(float3 iPos : POSITION, float4 iColor : COLOR,
	out float4 oPos : SV_POSITION, out float4 oColor : COLOR)
{
  
	oPos = mul(float4(iPos, 1.0f), gWorldViewProj);
	oColor = iColor;

}
