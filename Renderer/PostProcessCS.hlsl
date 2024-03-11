Texture2D g_sceneTex;
RWTexture2D<float4> g_dst;
SamplerState g_WrapSampler;

cbuffer Exporsure
{
    float g_exposure;
    float3 pad;
};

static const float _GAMMA = 2.2f;

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{   
    float2 size;
    g_sceneTex.GetDimensions(size.x, size.y);
    
    float3 hdrColor = g_sceneTex.SampleLevel(g_WrapSampler, (DTid.xy + 0.5) / (size - 1), 0).rgb;
    hdrColor = float3(1.0f, 1.0f, 1.0f) - exp(-hdrColor * g_exposure);
    hdrColor = pow(hdrColor, float3(1.0f / _GAMMA, 1.0f / _GAMMA, 1.0f / _GAMMA));
    
    g_dst[DTid.xy] = float4(hdrColor, 1.0f);
}