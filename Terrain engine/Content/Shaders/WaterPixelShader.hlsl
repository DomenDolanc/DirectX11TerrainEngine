Texture2D reflectionTexture : register(t0);
Texture2D refractionTexture : register(t1);
Texture2D dudvTexture : register(t2);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

static const float waveStrength = 0.02f;   
static const float4 waterTintColor = float4(0.0f, 0.3f, 0.5f, 1.0f);
static const float4 fogColor = float4(0.9f, 0.9f, 0.9f, 0.0f);
static const float fogStart = 23000.0f;
static const float fogRange = 8000.0f;

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 worldPos : POSITION0;
    float3 normal : NORMAL0;
    float3 vectorToCamera : POSITION1;
    float4 texCoord : TEXCOORD0;
    float2 dudvTexCoord : TEXCOORD1;
};

float4 main(PixelShaderInput input) : SV_TARGET
{    
    float2 ndc = (input.texCoord.xy / input.texCoord.w) / 2.0f + 0.5f;
    float2 reflectionTexCoord = float2(ndc.x, ndc.y);
    float2 refractionTexCoord = float2(ndc.x, -ndc.y);

    input.dudvTexCoord += waterMoveFactor;
    float2 distortion = (dudvTexture.Sample(simpleSampler, input.dudvTexCoord).rg * 2.0f - 1.0f) * waveStrength;

    reflectionTexCoord += distortion;
    reflectionTexCoord = clamp(reflectionTexCoord, 0.001f, 0.999f);
    
    refractionTexCoord += distortion;
    refractionTexCoord.x = clamp(refractionTexCoord.x, 0.001f, 0.999f);
    refractionTexCoord.y = clamp(refractionTexCoord.y, -0.999f, -0.001f);
    
    float3 viewVector = normalize(input.vectorToCamera);
    float refractiveFactor = dot(viewVector, input.normal);
    refractiveFactor = pow(refractiveFactor, 0.7f);
    
    float3 refractColor = refractionTexture.Sample(simpleSampler, refractionTexCoord).rgb;
    float3 reflectColor = reflectionTexture.Sample(simpleSampler, reflectionTexCoord).rgb;
    
    float4 combinedWaterColor = float4(lerp(reflectColor, refractColor, refractiveFactor), 1.0f);
    float4 finalWaterColor = lerp(combinedWaterColor, waterTintColor, 0.2f);
    float fogLerp = saturate((distance(input.worldPos, eyePos) - fogStart) / fogRange);
    return lerp(finalWaterColor, fogColor, fogLerp);
}
