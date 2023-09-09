Texture2D diffuseMap;
Texture2D normalMap;
Texture2D specularMap;
SamplerState g_defaultSampler : register(s0);
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 wPosition : POSITION0;
};
struct PixelOutputType
{
    float4 position : SV_Target0;
    float4 normal : SV_Target1;
    float4 albeldo : SV_Target2;
    float4 specular : SV_Target3;
};
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};
cbuffer cbMaterial
{
    Material gMaterial;
};
cbuffer PerModel
{
    bool isInstanced;
    bool hasNormal;
    bool hasSpecular;
    bool hasAlpha;
};
PixelOutputType main(PixelInputType input)
{   
    PixelOutputType output;
   
    output.albeldo = diffuseMap.Sample(g_defaultSampler, input.tex);
    clip(output.albeldo.a - 0.1f);
    output.position = input.wPosition;
    
    float3 normal = normalize(input.normal);
    [branch]
    if(hasNormal)
    {
    
        float3 tangent = normalize(input.tangent);
        float3 binormal = normalize(input.binormal);
        float3x3 TBN = float3x3(tangent, binormal, normal);
        normal = normalMap.Sample(g_defaultSampler, input.tex);
        normal = normal * 2.0 - 1.0;
        normal = mul(normal, TBN);
        normal = normalize(normal);
    }
    output.normal = float4(normal, 1.0f);
   
    output.specular.rgb = specularMap.SampleLevel(g_defaultSampler, input.tex,1);
    output.specular.a = 1.0f;

    
    
    
    
    return output;
}