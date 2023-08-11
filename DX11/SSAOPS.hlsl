Texture2D vPositionTex;
Texture2D vNormalTex;
Texture2D noiseTex;

SamplerState defaultState;
SamplerState clampState;

cbuffer cbSamplePoints : register(b0)
{
    float4 samplePoints[64];
};
cbuffer cbProjMat : register(b1)
{
    matrix projMat;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static const float radius = 0.5;
static const float bias = 0.025;
static const float2 noiseScale = float2(1584 / 4.0f, 761 / 4.0f);
static const int sampleCnt = 64;

float4 main(PixelInputType input) : SV_TARGET
{ 
    float3 vPos = vPositionTex.Sample(clampState, input.tex).xyz;
    float3 vNormal = normalize(vNormalTex.Sample(clampState, input.tex).xyz);
    float3 randomVec = normalize(noiseTex.Sample(defaultState, input.tex * noiseScale).xyz);
      
    float3 tangent = normalize(randomVec - vNormal * dot(randomVec, vNormal));
    float3 binormal = cross(vNormal, tangent);
    float3x3 TBN = float3x3(tangent, binormal, vNormal);
    
    float occlusion = 0.0f;
    [unroll]
    for (int i = 0; i < sampleCnt;++i)
    {
        float3 samplePos = mul(samplePoints[i].xyz, TBN);
        samplePos = vPos + samplePos * radius;
        
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(offset, projMat);
        offset.xyz /= offset.w;
        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = offset.y * -0.5f + 0.5f;
        
        float sampleDepth = vPositionTex.Sample(clampState, offset.xy).z;
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(vPos.z - sampleDepth));
        occlusion += (sampleDepth <= samplePos.z - bias ? 1.0f : 0.0f) * rangeCheck;
        //z축 방향 차이 
    }
 
    occlusion = 1.0f - (occlusion / sampleCnt);
    return occlusion;
}