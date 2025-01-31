Texture2D<float> g_lastMip;
SamplerState g_border;
RWTexture2D<float> g_dst;

cbuffer lastMipInfo
{
    uint lastWidth;
    uint lastHeight;
    uint lastMip;
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID) 
{   
    float4 texels;
    uint2 coord = DTid.xy* 2;
    texels.x = g_lastMip.Load(uint3(coord, 0)).x;
    texels.y = g_lastMip.Load(uint3(coord, 0), int2(-1, 0)).x;
    texels.z = g_lastMip.Load(uint3(coord, 0), int2(0, -1)).x;
    texels.w = g_lastMip.Load(uint3(coord,0), int2(-1, -1)).x;
    
    float maxZ = max(max(texels.x, texels.y), max(texels.z, texels.w));
    
    g_dst[DTid.xy] = maxZ;
}