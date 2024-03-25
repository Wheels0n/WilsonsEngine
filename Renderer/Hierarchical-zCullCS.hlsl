Texture2D<float> g_hi_zMap;
RWTexture2D<float> g_dst;
cbuffer vp
{   
    matrix view;
    matrix proj;
    matrix vpMat;
    float4 plane[6];
    float4 vPos;
    float4 vUp;
    float4 vDir;
};

cbuffer SphereInfo
{
    //xyz 중심, w 반지름
    float4 center[4096];
};

cbuffer resolution
{
    uint width;
    uint height;
};

SamplerState g_border;

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID,
        uint3 GroupID : SV_GroupID,
        uint GroupIndex : SV_GroupIndex)
{   
    uint idx = (GroupID.x + GroupID.y * 8) * 64 + GroupIndex;
    uint2 dst = uint2(idx % 64, idx / 64);
    
    float3 vEye = -view._m03_m13_m23;
    float camSphereDistance = distance(vEye, center[idx].xyz);
    
    float3 viewEyeSphereDir = vEye - center[idx].xyz;
    
    float3 up = view._m01_m11_m21;
    float3 dir = view._m02_m12_m22;
    float3 right = normalize(cross(viewEyeSphereDir, up));
    float radius = camSphereDistance * tan(asin(center[idx].w / camSphereDistance));
    
    float3 upRadius = up * radius;
    float3 rightRadius = right * radius;
    
    //WS
    float4 corner0 = float4(center[idx].xyz + upRadius - rightRadius, 1);
    float4 corner1 = float4(center[idx].xyz + upRadius + rightRadius, 1);
    float4 corner2 = float4(center[idx].xyz - upRadius - rightRadius, 1);
    float4 corner3 = float4(center[idx].xyz - upRadius + rightRadius, 1);
    
    //CS
    corner0 = mul(vpMat, corner0);
    corner1 = mul(vpMat, corner1);
    corner2 = mul(vpMat, corner2);
    corner3 = mul(vpMat, corner3);
    //NDC
    corner0.xyz /= corner0.w;
    corner1.xyz /= corner1.w;
    corner2.xyz /= corner2.w;
    corner3.xyz /= corner3.w;
    //Tex
    corner0.xy = float2(0.5f, -0.5f) * corner0.xy + float2(0.5f, 0.5f);
    corner1.xy = float2(0.5f, -0.5f) * corner1.xy + float2(0.5f, 0.5f);
    corner2.xy = float2(0.5f, -0.5f) * corner2.xy + float2(0.5f, 0.5f);
    corner3.xy = float2(0.5f, -0.5f) * corner3.xy + float2(0.5f, 0.5f);
    
    float sphererWidthNDC = distance(corner0.xy, corner1.xy);
    float3 centerV = mul(view, float4(center[idx].xyz, 1)).xyz;
    float3 centerP = centerV - normalize(centerV) * center[idx].w;
    float4 closestSpherePoint = mul(proj, float4(centerP, 1));
    
    float W = sphererWidthNDC * max(width, height);
    float lod = ceil(log2(W));
    
    
    float4 samples;
    samples.x = g_hi_zMap.SampleLevel(g_border, corner0.xy, lod);
    samples.y = g_hi_zMap.SampleLevel(g_border, corner1.xy, lod);
    samples.z = g_hi_zMap.SampleLevel(g_border, corner2.xy, lod);
    samples.w = g_hi_zMap.SampleLevel(g_border, corner3.xy, lod);
    
    float maxSampledDepth = max(max(samples.x, samples.y), max(samples.z, samples.w));
    float sphereDepth = closestSpherePoint.z / closestSpherePoint.w;
    
    g_dst[dst.xy] = sphereDepth>maxSampledDepth?0:1;
 
}