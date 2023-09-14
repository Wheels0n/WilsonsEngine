Texture2D g_diffuseMap;
Texture2D g_normalMap;
Texture2D g_specularMap;
Texture2D g_emissiveMap;
Texture2D g_alphaMap;
SamplerState g_sampler : register(s0);
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
    float4 emissive : SV_Target4;
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
    Material g_Material;
};
cbuffer PerModel
{
    bool g_isInstanced;
    bool g_hasNormal;
    bool g_hasSpecular;
    bool g_hasEmissive;
    bool g_hasAlpha;
    float3 pad;

};
PixelOutputType main(PixelInputType input)
{   
    PixelOutputType output;
  
    output.albeldo = g_diffuseMap.Sample(g_sampler, input.tex);
    clip(output.albeldo.a - 0.1f);
    output.position = input.wPosition;
    
    float3 normal = normalize(input.normal);
    [branch]
    if(g_hasNormal)
    {
    
        float3 tangent = normalize(input.tangent);
        float3 binormal = normalize(input.binormal);
        float3x3 TBN = float3x3(tangent, binormal, normal);
        normal = g_normalMap.Sample(g_sampler, input.tex);
        normal = normal * 2.0 - 1.0;
        normal = mul(normal, TBN);
        normal = normalize(normal);
    }
    output.normal = float4(normal, 1.0f);
   
    output.specular.rgb = g_specularMap.SampleLevel(g_sampler, input.tex,1);
    output.specular.a = 1.0f;
    
    
    output.emissive = float4(0.0f, 0.0f, 0.0f, 1.0f);
    [branch]
    if(g_hasEmissive)
    {
        output.emissive.xyz = g_emissiveMap.Sample(g_sampler, input.tex);
    }
    
    return output;
}