struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D g_lastMip;
SamplerState g_border;

cbuffer lastMipInfo
{
    uint lastWidth;
    uint lastHeight;
    uint lastMip;
};

float main(PixelInputType input) : SV_Target
{   
    float4 texels;
    uint2 coord = input.position.xy * 2;
    texels.x = g_lastMip.Load(uint3(coord, 0)).x;
    texels.y = g_lastMip.Load(uint3(coord, 0), int2(-1, 0)).x;
    texels.z = g_lastMip.Load(uint3(coord, 0), int2(0, -1)).x;
    texels.w = g_lastMip.Load(uint3(coord,0), int2(-1, -1)).x;
    
    float maxZ = max(max(texels.x, texels.y), max(texels.z, texels.w));
    
    return maxZ;
}