cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix mvp;
};

#include "IncludeDrawParams.hlsli"

struct VertexShaderInput
{
    float3 pos : POSITION;
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

static const float tilingFactor = scaling / 10000.0f;

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    
    output.normal = float3(0.0f, 1.0f, 0.0f);
    output.worldPos = input.pos;
    output.vectorToCamera = eyePos - pos.xyz;
    output.dudvTexCoord = (pos.xz / scaling + 0.5f) * tilingFactor;
    
    output.texCoord = mul(pos, mvp);
    
    pos = mul(pos, mvp);
    output.pos = pos;

    return output;
}