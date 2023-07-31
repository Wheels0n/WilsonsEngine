struct VertexInputType
{
    float3 position : POSITION;
};
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightSpaceMat;
};

float4 main(VertexInputType input) :SV_Position
{   
    float4 output = float4(input.position, 1.0f);
    output = mul(output, worldMatrix);
    output = mul(output, viewMatrix);
    output = mul(output, projectionMatrix);
    
    return output;
}