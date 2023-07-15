struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
 
};

cbuffer lightPos
{
    float4 lightPos;//W is Far_Plane
};
struct PS_OUTPUT
{   
    float color : SV_Target;
    float depth : SV_Depth;
};

PS_OUTPUT main(PS_INPUT input )
{       
    PS_OUTPUT output;
    output.depth = length(input.wPos.xyz - lightPos.xyz);
    output.depth = output.depth/lightPos.w;
    output.color = float4(output.depth, output.depth, output.depth, 1.0f);
    return output;
}