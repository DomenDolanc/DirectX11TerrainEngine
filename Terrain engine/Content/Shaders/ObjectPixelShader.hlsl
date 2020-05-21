Texture2D objectTexture : register(t0);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL0;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : POSITION0;
};

static const float3 ambient = float3(0.202f, 0.233f, 0.292f);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = objectTexture.Sample(simpleSampler, input.texCoord);
    float diffuse = saturate(dot(input.normal, normalize(lightPos)));
    return float4(saturate((color.rgb * diffuse) + (color.rgb * ambient)), color.a);
}