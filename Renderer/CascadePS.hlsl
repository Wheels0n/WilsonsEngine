struct GSOutput
{
    float4 pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};
[earlydepthstencil]
float4 main(GSOutput input) : SV_Target
{
    float depth = input.pos.z;
    return float4(depth, depth, depth, 1.0f);
}