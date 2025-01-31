Texture2D g_fullScreenDepth;
RWTexture2D<float> g_dst;
SamplerState g_borderSampler;

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 dimension;
    g_dst.GetDimensions(dimension.x, dimension.y);
    g_dst[DTid.xy] = g_fullScreenDepth.SampleLevel(g_borderSampler, (float2) DTid.xy / dimension, 0);

}