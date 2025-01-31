Texture2D<float> g_hi_zMap;
RWTexture2D<uint> g_dst;

cbuffer resolution : register(b0)
{
    uint width;
    uint height;
};

cbuffer vertex : register(b1)
{   
 
    float4 v[256][8];
};

cbuffer wvp : register(b2)
{
 
    float4x4 wvp[256];
};

SamplerState g_border;
void ToTexCoord(uint idx, out float2 minV, out float2 maxV, out float depth)
{
    depth = 1.0f;
    minV = float2(1.0f, 1.0f);
    maxV = float2(0.0f, 0.0f);
    [rollout]
    for (int i = 0; i < 8; ++i)
    {
        float4 ndc = mul(v[idx][i], wvp[idx]);
        ndc /= ndc.w;
        float2 texCoord = ndc.xy;
        texCoord *= float2(0.5f, -0.5f);
        texCoord += 0.5f;
        texCoord.x = clamp(texCoord.x, 0.0f, 1.0f);
        texCoord.y = clamp(texCoord.y, 0.0f, 1.0f);
        minV = min(texCoord, minV);
        maxV = max(texCoord, maxV);
        depth = min(ndc.z, depth);
    }
   
}

[numthreads(16, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{   
    uint idx = DTid.x;
    uint2 dst = uint2(DTid.x % 16, DTid.x /16);
    
    float2 minV = float2(1.0f, 1.0f);
    float2 maxV = float2(0.0f, 0.0f);
    float depth = 1.0f;
    ToTexCoord(idx, minV, maxV, depth);
    
    float2 sc = (maxV - minV) * float2(width, height);
    
    float lod = ceil(log2(max(sc.x, sc.y)/2));
    float4 samples;
   
    samples.x = g_hi_zMap.SampleLevel(g_border, float2(maxV.x, maxV.y), lod);
    samples.y = g_hi_zMap.SampleLevel(g_border, float2(minV.x, maxV.y), lod);
    samples.z = g_hi_zMap.SampleLevel(g_border, float2(maxV.x, minV.y), lod);
    samples.w = g_hi_zMap.SampleLevel(g_border, float2(minV.x, minV.y), lod);
    
    float maxSampledDepth = max(max(samples.x, samples.y), max(samples.z, samples.w));
    
    g_dst[dst.xy] = depth > maxSampledDepth ? 0 : 1;
 
}