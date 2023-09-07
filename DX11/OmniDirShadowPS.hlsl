struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    float2 tex : TEXTURE;
    uint RTIndex : SV_RenderTargetArrayIndex;
 
};
struct PS_OUTPUT
{
    float color : SV_Target;
    float depth : SV_Depth;
};
cbuffer lightPos
{
    float4 lightPos;//W is Far_Plane
};
Texture2D abeldoTex;
SamplerState SampleType : register(s0);
PS_OUTPUT main(PS_INPUT input )
{       
    PS_OUTPUT output;
    float4 albedo =abeldoTex.Sample(SampleType, input.tex);
    clip(albedo.a - 0.1f);
    
    output.depth = length(input.wPos.xyz - lightPos.xyz);
    output.depth = output.depth/lightPos.w;
    output.color = float4(output.depth, output.depth, output.depth, 1.0f);
    return output;
}