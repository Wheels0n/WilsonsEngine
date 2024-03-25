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
};

float main(PixelInputType input) : SV_Target
{   
    float4 texels;
    float2 texelSize = float2((float) 1 / lastWidth, (float) 1 / lastHeight);
    texels.x = g_lastMip.Sample(g_border, input.tex).x;
    texels.y = g_lastMip.Sample(g_border, input.tex + float2(-1, 0) * texelSize).x;
    texels.z = g_lastMip.Sample(g_border, input.tex + float2(-1, -1) * texelSize).x;
    texels.w = g_lastMip.Sample(g_border, input.tex + float2(0, -1) * texelSize).x;
    
    float maxZ = max(max(texels.x, texels.y), max(texels.z, texels.w));
    
    return maxZ;
}