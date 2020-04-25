cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};


#include "IncludeDrawParams.hlsli"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 normal : NORMAL0;
    float3 vectorToCamera : POSITION1;
    float4 texCoord : TEXCOORD0;
    float2 dudvTexCoord : TEXCOORD1;
};

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    
    output.normal = input.normal;
    output.worldPos = input.pos;
    output.vectorToCamera = eyePos - pos.xyz;
    output.dudvTexCoord = (pos.xz / scaling + 0.5f) * 6.0f;
    
    output.texCoord = mul(pos, model);
    output.texCoord = mul(output.texCoord, view);
    output.texCoord = mul(output.texCoord, projection);
    
    pos = mul(pos, model);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.pos = pos;

    return output;
}