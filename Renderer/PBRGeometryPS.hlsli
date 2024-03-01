Texture2D g_diffuseMap : register(t0);
Texture2D g_normalMap : register(t1);
Texture2D g_specularMap : register(t2);
Texture2D g_emissiveMap : register(t3);
Texture2D g_alphaMap : register(t4);
SamplerState g_WrapSampler : register(s0);
struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 wPosition : POSITION0;
    float4 vPosition : POSITION1;
    float3 vNormal : vNORMAL;
};
struct PixelOutputType
{
    float4 position : SV_Target;
    float4 normal : SV_Target1;
    float4 albeldo : SV_Target2;
    float4 specular : SV_Target3;
    float4 emissive : SV_Target4;
    float4 vPos : SV_Target5;
    float4 vNormal : SV_Target6;
    float4 depth : SV_Target7;
};
struct Material
{
    float4 ambient;
    float4 diffuse;
    float4 specular; // w = SpecPower
    float4 reflect;
};

cbuffer CamBuffer : register(b0)
{
    float4 g_camPos;
};

cbuffer cbMaterial : register(b1)
{
    Material g_Material;
};

cbuffer heightScale : register(b2)
{
    float g_heightScale;
    float3 pad;
};

static const float g_minLayers = 8;
static const float g_maxLayers = 128;

float2 ParallaxOcclusionMapping(float2 texCoord, float3 viewDir)
{
    int numLayer = (int) lerp(g_maxLayers, g_minLayers, abs(dot(float3(0.0, 0.0, 1.0f), viewDir)));
    float layerDepth = 1.0f / (float) numLayer;
    float curLayerDepth = 0.0f;
    float2 p = viewDir.xy / viewDir.z * g_heightScale;
    
    float2 dTexCoord = p / numLayer;
    
    float2 curTexCoord = texCoord;
    float curDepthMapVal = g_normalMap.Sample(g_WrapSampler, texCoord).a - 1.0f;
    
    for (int i = 0; i < g_maxLayers; ++i)
    {
        if (curLayerDepth < curDepthMapVal)
        {
            break;
        }
        curTexCoord -= dTexCoord;
        curDepthMapVal = g_normalMap.Sample(g_WrapSampler, curTexCoord).a - 1.0f;
        curLayerDepth -= layerDepth;
        
    }
    float2 prevTexCoord = curTexCoord + dTexCoord;
    
    float afterDepth = curDepthMapVal - curLayerDepth; //¨úc   //A¨ö
    float beforeDepth = (g_normalMap.Sample(g_WrapSampler, prevTexCoord).a - 1.0f) - (curLayerDepth + layerDepth);
    
    float weight = afterDepth / (afterDepth - beforeDepth);
    
    return prevTexCoord * weight + curTexCoord * (1.0f - weight);
    
    
}
PixelOutputType main(PixelInputType input)
{
    PixelOutputType output;
    float3 normal = normalize(input.normal);
    float2 texCoord = input.tex;
#ifdef HAS_NORMAL 
    {
    
        float3 tangent = normalize(input.tangent);
        float3 binormal = normalize(input.binormal);
        float3x3 TBN = (float3x3(tangent, binormal, normal));
        
        float3 viewDir = normalize(g_camPos - input.wPosition);
        viewDir = mul(viewDir, TBN);
        viewDir = normalize(viewDir);
#ifdef HAS_HEIGHT 
        {
        
            texCoord = ParallaxOcclusionMapping(input.tex, viewDir);
            [branch]
            if (texCoord.x > 1.0f || texCoord.x < 0.0f || texCoord.y > 1.0f || texCoord.y < 0.0f)
            {
                clip(-1);
            }
        }
#endif         
        normal = g_normalMap.Sample(g_WrapSampler, texCoord);
        normal = normal * 2.0 - 1.0;
        normal = mul(normal, TBN);
        normal = normalize(normal);
     
    }
#endif   
    output.normal = float4(normal, 1.0f);
    
    output.albeldo = g_diffuseMap.Sample(g_WrapSampler, texCoord);
    clip(output.albeldo.a - 0.1f);
    output.position = input.wPosition;
    output.vPos = input.vPosition;
    
    float depth = output.vPos.z / 100.0f;
    output.depth = float4(depth, depth, depth, 1.0f);
    output.vNormal = float4(normalize(input.vNormal), 1.0f);

   
    output.specular.rgb = g_specularMap.SampleLevel(g_WrapSampler, texCoord, 1);
    output.specular.a = 1.0f;
    
#ifdef HAS_EMSSIVE
     output.emissive.xyz = g_emissiveMap.Sample(g_WrapSampler, texCoord);
#else
    output.emissive = float4(0.0f, 0.0f, 0.0f, 1.0f);
#endif
    return output;
}