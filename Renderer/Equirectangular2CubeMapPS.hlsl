struct PixelInputType
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

Texture2D g_equirectanglarMap;
SamplerState g_sampler;

static const float _GAMMA = 2.2f;
static const float2 _INV_ATAN = float2(0.1591, 0.3183);// 1/2PI, 1/PI  
float2 SampleSphericalMap(float3 v)
{   //������ǥ�迡�� UV��ǥ�� �ٲٱ� ����  ����  ���� ���� �����ϰ� �̸� �ٽ� Unwrap�Ѵ�
    //�� ���� 3���� ��ǥ�� phi, theta�� ã�� �̸� ���� ���뿡 ����� ��ǥ�� ã�´�
    //glsl�� �����ε� �Ǿ������� ���⼭�� atan2�� ����. 
    float2 uv = float2(atan2(v.z, v.x), asin(v.y)); //phi, theta
    uv *= _INV_ATAN;
    uv += float2(0.5f, 0.5f);
    uv.y = 1.0f - uv.y;
    return uv;
}
float4 main(PixelInputType input) : SV_TARGET
{   
    float2 uv = SampleSphericalMap(normalize(input.wPos.xyz));
    float3 color = g_equirectanglarMap.Sample(g_sampler, uv).rgb;
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(1.0f / _GAMMA, 1.0f / _GAMMA, 1.0f / _GAMMA));
    return float4(color, 1.0f);
}