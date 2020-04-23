Texture2D reflectionTexture : register(t0);
Texture2D dudvTexture : register(t1);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

static const float waveStrength = 0.02f;    

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
    float4 texCoord : TEXCOORD0;
    float2 dudvTexCoord : TEXCOORD1;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    input.texCoord.x = (input.texCoord.x / input.texCoord.w) / 2.0f + 0.5f;
    input.texCoord.y = (input.texCoord.y / input.texCoord.w) / 2.0f + 0.5f;

    input.dudvTexCoord += waterMoveFactor;
    float2 distortion = (dudvTexture.Sample(simpleSampler, input.dudvTexCoord).rg * 2.0f - 1.0f) * waveStrength;

    input.texCoord.xy += distortion;
    
    float3 sampledColor = reflectionTexture.Sample(simpleSampler, input.texCoord.xy).rgb;
    
    
    return float4(lerp(float3(0.0f, 0.5f, 1.0f), sampledColor, 0.5f), 0.8f);
}
