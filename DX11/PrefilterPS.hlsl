struct PixelInputType
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

TextureCube g_envMap;
SamplerState g_sampler;
cbuffer roughness
{
    float g_roughness;
    float3 pad;
};

static const float _PI = 3.14159265359f;

float DistributionGGX(float3 normal, float3 h, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(normal, h), 0.0f);
    float NdotH2 = NdotH * NdotH;
    
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom = _PI * denom * denom;
    
    return nom / denom;
}
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 normal, float roughness)
{
    float a = roughness * roughness;
    
    float phi = 2.0f * _PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0f + (a * a - 1.0f) * Xi.y));
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    
    float3 h;
    h.x = cos(phi) * sinTheta;
    h.y = sin(phi) * sinTheta;
    h.z = cosTheta;
    
    float3 up = abs(normal.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, normal));
    float3 bitangent = cross(normal, tangent);
    
    float3 sampleVec = tangent * h.x + bitangent * h.y + normal * h.z;
    return normalize(sampleVec);

}

float4 main(PixelInputType input) : SV_TARGET
{
    float3 normal = normalize(input.wPos);
    float3 R = normal;
    float3 V = R;
    
    const uint SAMPLE_CNT = 1024U;
    float3 prefilteredColor = float3(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    
    for (uint i = 0u; i < SAMPLE_CNT;++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_CNT);
        float3 H = ImportanceSampleGGX(Xi, normal, g_roughness);
        float3 L = normalize(2.0f * dot(V, H) * H - V);
        
        float NdotL = max(dot(normal, L), 0.0f);
        if(NdotL>0.0f)
        {
            float D = DistributionGGX(normal, H, g_roughness);
            float NdotH = max(dot(normal, H), 0.0f);
            float HdotV = max(dot(H, V), 0.0f);
            float pdf = D * NdotH / (4.0f * HdotV) + 0.0001;
            
            float resolution = 1024.0f;
            float saTexel = 4.0f * _PI / (6.0F * resolution * resolution);
            float saSample = 1.0f / (float(SAMPLE_CNT) * pdf + 0.0001f);
            
            float mipLevel = g_roughness == 0.0f ? 0.0f : 0.5 * log2(saSample / saTexel);
            
            prefilteredColor += g_envMap.SampleLevel(g_sampler, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }
    prefilteredColor = prefilteredColor / totalWeight;
    return float4(prefilteredColor, 1.0f);
}