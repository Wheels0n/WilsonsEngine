Texture2D g_noiseTex;
Texture2D g_vPosTex;
Texture2D g_vNormalTex;
RWTexture2D<float4> g_Dst;

SamplerState g_WrapSampler;
SamplerState g_clampSampler;

static const int _NoiseTex_Len = 4;

cbuffer Kernels : register(b0)
{
    float4 g_kernels[64];
};
cbuffer ProjMat : register(b1)
{
    matrix g_projMat;
};
cbuffer SSAOParameters : register(b2)
{
    float _RADIUS;
    float _BIAS;
    uint _SAMPLE_CNT;
};


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
{   
    float2 size;
    g_Dst.GetDimensions(size.x, size.y);
    float2 uv = (DTid.xy / size);
    
    float4 vPos = g_vPosTex.SampleLevel(g_clampSampler, uv, 0);
    if (dot(vPos.xyz, vPos.xyz) == 0)
    {
        g_Dst[DTid.xy] = 1.0f;
        return;
    }
    
    float2 _NOISE_SCALE = float2(size.x / _NoiseTex_Len, size.y / _NoiseTex_Len);
    float3 randomVec = normalize(g_noiseTex.SampleLevel(g_WrapSampler, uv * _NOISE_SCALE,0).xyz);
   
    float3 vNormal = g_vNormalTex.SampleLevel(g_clampSampler, uv, 0);
   
    float3 tangent = normalize(randomVec - vNormal * dot(randomVec, vNormal));
    float3 binormal = cross(vNormal, tangent);
    float3x3 TBN = float3x3(tangent, binormal, vNormal);
    
    float occlusion = 0.0f;
    
    [unroll]
    for (int i = 0; i < _SAMPLE_CNT; ++i)
    {   
        float3 samplePos = mul(g_kernels[i].xyz, TBN);
        samplePos = vPos.xyz + samplePos * _RADIUS;
        
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(offset, g_projMat);
        offset.xyz /= offset.w;
        //ndc(x,y는 -1~1)->tex
        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = offset.y * -0.5f + 0.5f;
        
        float4 sampleDepth = g_vPosTex.SampleLevel(g_clampSampler, offset.xy, 0);
        float rangeCheck = smoothstep(0.0f, 1.0f, _RADIUS / abs(vPos.z - sampleDepth.z));
        occlusion += (sampleDepth.z <= samplePos.z - _BIAS ? 1.0f : 0.0f) * rangeCheck;
        //z축 방향 차이 
    }
   
    occlusion = 1-(occlusion / _SAMPLE_CNT);
    
    g_Dst[DTid.xy] = occlusion;
}