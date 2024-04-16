Texture2D<float> g_hi_zMap;
RWTexture2D<uint> g_dst;
cbuffer vp : register(b0)
{   
    matrix view;
    matrix proj;
    matrix vpMat;
    float4 plane[6];
    float4 vPos;
    float4 vUp;
    float4 vDir;
};

cbuffer resolution : register(b1)
{
    uint width;
    uint height;
};

cbuffer BoundInfo : register(b2)
{   
    //Tex최소xy, 최대xy
    float4 bound[4096];
};

cbuffer Depth : register(b3)
{
    //[0,1]
    float depth[4096];
};

SamplerState g_border;

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{   
    uint idx = DTid.x;
    uint2 dst = uint2(DTid.x % 64, DTid.x / 64);
    
    float2 sc = (bound[idx].zw - bound[idx].xy) * float2(width, height);
    
    float lod = ceil(log2(max(sc.x, sc.y)));
    clamp(lod, 0, log2(width));
    float level_lower = max(lod - 1, 0);
    float2 scale = exp2(-level_lower).xx;
    float2 dims = float2(ceil(scale * bound[idx].zw) - floor(scale * bound[idx].xy));
    if(dims.x<=2&&dims.y<=2)
    {
        lod = level_lower;
    }

    
    float4 samples;
   
    samples.x = g_hi_zMap.SampleLevel(g_border, bound[idx].xy, lod);
    samples.y = g_hi_zMap.SampleLevel(g_border, bound[idx].xw, lod);
    samples.z = g_hi_zMap.SampleLevel(g_border, bound[idx].zy, lod);
    samples.w = g_hi_zMap.SampleLevel(g_border, bound[idx].zw, lod);
    
    float minSampledDepth = max(max(samples.x, samples.y), max(samples.z, samples.w));
    
    g_dst[dst.xy] = depth[idx] > minSampledDepth ? 0 : 1;
 
}