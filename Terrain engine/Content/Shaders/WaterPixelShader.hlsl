Texture2D reflectionTexture : register(t0);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
    float4 texCoord : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    //return float4(1.0f, 1.0f, 1.0f, 1.0f);
    input.texCoord.x = (input.texCoord.x / input.texCoord.w) / 2.0f + 0.5f;
    input.texCoord.y = (input.texCoord.y / input.texCoord.w) / 2.0f + 0.5f;
    //if (texCoord.x >= 0.7f)
    //    return float4(1.0f, 1.0f, 1.0f, 1.0f);
    //else
    //    return float4(0.0f, 0.0f, 0.0f, 1.0f);
    float3 sampledColor = reflectionTexture.Sample(simpleSampler, input.texCoord.xy).rgb;
    //return float4(sampledColor, 1.0f);
    return float4(lerp(float3(0.0f, 0.0f, 1.0f), sampledColor, 0.5f), 0.8f);
}
