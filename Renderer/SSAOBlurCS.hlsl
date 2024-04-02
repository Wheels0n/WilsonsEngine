RWTexture2D<float4> g_Dst;
RWTexture2D<float4> g_Debug;
Texture2D g_SSAOTex;
SamplerState g_WrapSampler;

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
{
    float2 size;
    g_Dst.GetDimensions(size.x, size.y);
    
    if (DTid.x >= size.x || DTid.y >= size.y)
    {
        return;
    }
    float2 texelSize = 1 / size;
    float2 uv = (DTid.xy + 0.5)/size;
    float result = 0.0f;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            result += g_SSAOTex.SampleLevel(g_WrapSampler, uv.xy + offset, 0);
        }

    }
    
    result /= (4.0f * 4.0f);
    g_Dst[DTid.xy] = result;
    g_Debug[DTid.xy] = float4(result, result, result, 1.0f);
}