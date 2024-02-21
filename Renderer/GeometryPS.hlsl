Texture2D g_diffuseMap;
Texture2D g_specularMap;
Texture2D g_normalMap;
Texture2D g_alphaMap;
SamplerState g_sampler : register(s0);
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 vNormal : NORMAL1;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 wPosition : POSITION0;
    float4 vPosition : POSITIONT1;
};

struct PixelOutputType
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albeldo : SV_Target2;
    float4 specular : SV_Target3;
    float4 vPos : SV_Target4;
    float4 vNormal : SV_Target5;
    float4 reflectivity : SV_Target6;
};

struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};
cbuffer Material
{
    Material g_Material;
};
cbuffer PerModel
{
    bool g_isInstanced;
    bool g_hasNormal;
    bool g_hasSpecular;
    bool g_hasAlpha;
};

PixelOutputType main(PixelInputType input)  
{
    PixelOutputType output;

    float4 alphaIntensity = float4(1.0f, 1.0f, 1.0f, 1.0f);
    
    [branch]
    if (g_hasAlpha)
    {
        alphaIntensity = g_alphaMap.Sample(g_sampler, input.tex);
        clip(dot((float3) alphaIntensity, (float3) alphaIntensity) - 0.1f);
    }
    
    output.position = input.wPosition;
    output.vPos = input.vPosition;
    
    float3 normal = normalize(input.normal);
    [branch]
    if (g_hasNormal)
    {
        float3 tangent = normalize(input.tangent);
        float3 binormal = normalize(input.binormal);
        float3x3 TBN = float3x3(tangent, binormal, normal);
        normal = g_normalMap.Sample(g_sampler, input.tex);
        normal = normal * 2.0 - 1.0;
        normal = mul(normal, TBN);
        normal = normalize(normal);
    }
    output.normal = float4(normal,1.0f);
    output.vNormal = float4(normalize(input.vNormal), 1.0f);
    
    output.albeldo = g_diffuseMap.Sample(g_sampler, input.tex);
    clip(output.albeldo.a - 0.1f);
    
    output.specular = g_Material.specular;
    [branch]
    if (g_hasSpecular)
    {
        output.specular.rgb = g_specularMap.Sample(g_sampler, input.tex);
    }
    output.reflectivity = g_Material.reflect;
    
	return output;
}