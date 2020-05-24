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
    return skyboxTexture.Sample(simpleSampler, input.texCoord);
}
