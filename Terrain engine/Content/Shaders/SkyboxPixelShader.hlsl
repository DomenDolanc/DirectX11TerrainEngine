TextureCube skyboxTexture : register(t0);
SamplerState simpleSampler;

#include "IncludeWaterParams.hlsli"

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = skyboxTexture.Sample(simpleSampler, input.texCoord);
    return float4((color.rgb - 0.5f) * (1.0f + 0.3f) + 0.5f, color.a);
}
