TextureCube g_SrcMip;
RWTexture2DArray<float4> g_DstMip;
SamplerState g_BilinerClamp;

cbuffer MipInfo
{   
    float2 TexelSize;
};

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = (DTid.xy+0.5) / TexelSize;
    uv *= 2.0f;
    uv -= 1.0f;
    
    float3 dir;
    float4 color = 0.0f;
   
    switch (DTid.z)
    { //https://blog.naver.com/sorkelf/40171465780
        //X+
        case 0:
            dir.x = 1.0f;
            dir.y = -uv.y;
            dir.z = -uv.x;
            break;
        //X-
        case 1:
            dir.x = -1.0f;
            dir.y = -uv.y;
            dir.z = uv.x;
            break;
        //Y+
        case 2:
            dir.x = uv.x;
            dir.y = 1.0f;
            dir.z = uv.y;
            break;
        //Y-
        case 3:
            dir.x = uv.x;
            dir.y = -1.0f;
            dir.z = -uv.y;
            break;
        //Z+
        case 4:
            dir.x = uv.x;
            dir.y = uv.y;
            dir.z = 1.0f;
            break;
        case 5:
            dir.x = -uv.x;
            dir.y = -uv.y;
            dir.z = -1.0f;
            break;
    }
    
    color = g_SrcMip.SampleLevel(g_BilinerClamp, dir, 0);
    g_DstMip[DTid.xyz] = color;
}