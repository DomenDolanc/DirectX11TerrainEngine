TextureCube skyboxTexture : register(t0);
SamplerState simpleSampler;

#include "IncludeWaterParams.hlsli"

static const float4 fogColor = float4(0.9f, 0.9f, 0.9f, 0.0f);
static const float fogStart = 23000.0f;
static const float fogRange = 8000.0f;

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 texCoord : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 skyColor = skyboxTexture.Sample(simpleSampler, input.texCoord);
    float fogLerp = saturate((distance(input.worldPos, eyePos) - fogStart) / fogRange);
    return skyColor; //lerp(skyColor, fogColor, fogLerp);
}
