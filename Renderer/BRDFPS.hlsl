struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

static const float _PI = 3.14159265359f;

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
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness;
    float k = (r * r) / 2.0f;
    
    float nom = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;

}

float GeometrySmith(float3 normal, float3 viewDir, float3 lightDir, float roughness)
{
    float NdotV = max(dot(normal, viewDir), 0.0f);
    float NdotL = max(dot(normal, lightDir), 0.0f);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}
float2 IntergrateBRDF(float NdotV,float r)
{
    float3 V;
    V.x = sqrt(1.0f - NdotV * NdotV);
    V.y = 0.0f;
    V.z = NdotV;
    
    float A = 0.0f;
    float B = A;
    
    float3 normal = float3(0.0f, 0.0f, 1.0f);
    
    const uint SAMPLE_CNT = 1024U;
    for (uint i = 0u; i < SAMPLE_CNT;++i)
    {
        float2 Xi = Hammersley(i, SAMPLE_CNT);
        float3 H = ImportanceSampleGGX(Xi, normal, r);
        float3 L = normalize(2.0f * dot(V, H) * H - V);
        
        float NdotL = max(L.z, 0.0f);
        float NdotH = max(H.z, 0.0f);
        float VdotH = max(dot(V, H), 0.0f);
        
        if(NdotL>0.0f)
        {
            float G = GeometrySmith(normal, V, L, r);
            float G_Vis = (G * VdotH) / (NdotH * NdotV);
            float Fc = pow(1.0f - VdotH, 5.0f);
            
            A += (1.0f - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }

    A /= float(SAMPLE_CNT);
    B /= float(SAMPLE_CNT);
    return float2(A, B);
}


float2 main(PixelInputType input) : SV_TARGET
{    
    return IntergrateBRDF(input.tex.x ,1.0f-input.tex.y); 
}