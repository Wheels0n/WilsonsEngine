struct PixelInputType
{
    float4 pos : SV_POSITION;
    float4 wPos : POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

TextureCube g_envMap;
SamplerState g_cubeSampler;

static const float _PI = 3.14159265359;
float4 main(PixelInputType input) : SV_TARGET
{
    float3 normal = normalize(input.wPos.xyz);
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    
    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 right = normalize(cross(up, normal));
    up = (cross(normal, right));
    
    int nrSamples = 0;
    for (float phi = 0.0; phi < 2.0 * _PI; phi+=0.025f)
    {
        for (float theta = 0.0f; theta < 0.5 * _PI; theta+=0.1f)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec=tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
           
            irradiance += g_envMap.Sample(g_cubeSampler, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }

    }
    irradiance = _PI * irradiance/nrSamples;
    return float4(irradiance, 1.0f);

}