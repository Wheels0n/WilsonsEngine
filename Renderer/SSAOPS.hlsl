Texture2D g_depthTex;
Texture2D g_noiseTex;

SamplerState g_WrapSampler;
SamplerState g_clampSampler;

static const float _RADIUS = 0.5;
static const float _BIAS = 0.025;
static const int _SAMPLE_CNT = 64;
static const int _NoiseTex_Len=4;

cbuffer Kernels : register(b0)
{
    float4 g_kernels[_SAMPLE_CNT];
};
cbuffer ProjMat : register(b1)
{
    matrix g_projMat;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float2 viewRay : ViewRay;
};


float CalViewZ(float2 tex)
{
    float viewZ=0.0f;
    float ndcZ = g_depthTex.Sample(g_WrapSampler, tex).r;
    viewZ = g_projMat[3][2]/(ndcZ - g_projMat[2][2]);
    
    return viewZ;
}

float4 main(PixelInputType input) : SV_TARGET
{   
    float sizeX, sizeY;
    g_depthTex.GetDimensions(sizeX, sizeY);
    float2 _NOISE_SCALE = float2(sizeX / _NoiseTex_Len, sizeY / _NoiseTex_Len);
    
    float3 vPos = float3(0, 0, 0);
    vPos.z = CalViewZ(input.tex);
    vPos.x = vPos.z * input.viewRay.x;
    vPos.y = vPos.z * input.viewRay.y;
    
    float3 randomVec = normalize(g_noiseTex.Sample(g_WrapSampler, input.tex * _NOISE_SCALE).xyz);
    
    float occlusion = 0.0f;
    [unroll]
    for (int i = 0; i < _SAMPLE_CNT;++i)
    {
        float3 samplePos = vPos + g_kernels[i].xyz * _RADIUS;
        
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(offset, g_projMat);
        offset.xyz /= offset.w;
        //ndc->tex(x,y는 -1~1)
        offset.x = offset.x * 0.5f + 0.5f;
        offset.y = offset.y * -0.5f + 0.5f;
        
        float sampleDepth = CalViewZ(offset.xy);
        float rangeCheck = smoothstep(0.0f, 1.0f, _RADIUS / abs(vPos.z - sampleDepth));
        occlusion += (sampleDepth <= vPos.z - _BIAS ? 1.0f : 0.0f) * rangeCheck;
        //z축 방향 차이 
    }
 
    occlusion = 1.0f - (occlusion / _SAMPLE_CNT);
    return occlusion;
}