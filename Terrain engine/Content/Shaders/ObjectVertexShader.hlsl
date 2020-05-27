cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix mvp;
};

#include "IncludeDrawParams.hlsli"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
};

struct VertexShaderOutput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : POSITION0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    
    float4 pos = float4(input.pos * 100, 1.0f);
    output.worldPos = pos.xyz;
    output.normal = input.normal;
    output.texCoord = input.texCoord;


    pos = mul(pos, mvp);
    output.pos = pos;
    
    return output;
}
