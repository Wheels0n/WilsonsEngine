Texture2D g_vPositionTex;
Texture2D g_vNormalTex;
Texture2D g_noiseTex;

SamplerState g_WrapSampler;
SamplerState g_clampSampler;

static const float _RADIUS = 0.5;
static const float _BIAS = 0.025;
static const int _SAMPLE_CNT = 64;
static const int _NoiseTex_Len=4;

cbuffer SamplePoints : register(b0)
{
    float4 g_samplePoints[_SAMPLE_CNT];
};
cbuffer ProjMat : register(b1)
{
    matrix g_projMat;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};


float4 main(PixelInputType input) : SV_TARGET
{   
    float sizeX, sizeY;
    g_vPositionTex.GetDimensions(sizeX, sizeY);
    float2 _NOISE_SCALE = float2(sizeX / _NoiseTex_Len,    sizeY / _NoiseTex_Len);
    
    float3 vPos = g_vPositionTex.Sample(g_clampSampler, input.tex).xyz;
    float3 vNormal = normalize(g_vNormalTex.Sample(g_clampSampler, input.tex).xyz);
    float3 randomVec = normalize(g_noiseTex.Sample(g_WrapSampler, input.tex * _NOISE_SCALE).xyz);
      
    float3 tangent = normalize(randomVec - vNormal * dot(randomVec, vNormal));
    float3 binormal = cross(vNormal, tangent);
    float3x3 TBN = float3x3(tangent, binormal, vNormal);
    
    float occlusion = 0.0f;
    [unroll]
    for (int i = 0; i < _SAMPLE_CNT;++i)
    {
        float3 samplePos = mul(g_samplePoints[i].xyz, TBN);
        samplePos = vPos + samplePos * _RADIUS;
        
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(offset, g_projMat);
        offset.xyz /= offset.w;
        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = offset.y * -0.5f + 0.5f;
        
        float sampleDepth = g_vPositionTex.Sample(g_clampSampler, offset.xy).z;
        float rangeCheck = smoothstep(0.0f, 1.0f, _RADIUS / abs(vPos.z - sampleDepth));
        occlusion += (sampleDepth <= samplePos.z - _BIAS ? 1.0f : 0.0f) * rangeCheck;
        //z�� ���� ���� 
    }
 
    occlusion = 1.0f - (occlusion / _SAMPLE_CNT);
    return occlusion;
}