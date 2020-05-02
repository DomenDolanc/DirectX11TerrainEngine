Texture2D reflectionTexture : register(t0);
Texture2D refractionTexture : register(t1);
Texture2D dudvTexture : register(t2);
SamplerState simpleSampler;

#include "IncludeWaterParams.hlsli"
#include "IncludeFogParams.hlsli"
 
static const float4 waterTintColor = float4(0.0f, 0.6f, 0.8f, 1.0f);

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
    if (reflectWater == 0.0f && refractWater == 0.0f)
        return waterTintColor;
    
    float2 ndc = (input.texCoord.xy / input.texCoord.w) / 2.0f + 0.5f;
    float2 reflectionTexCoord = float2(ndc.x, ndc.y);
    float2 refractionTexCoord = float2(ndc.x, -ndc.y);
    
    float3 viewVector = normalize(input.vectorToCamera);
    float refractiveFactor = saturate(dot(viewVector, input.normal));
    refractiveFactor = pow(refractiveFactor, 1.4f);

    input.dudvTexCoord += waterMoveFactor;
    float2 distortion = (dudvTexture.Sample(simpleSampler, input.dudvTexCoord).rg * 2.0f - 1.0f) * waterStrengthFactor;

    float3 refractColor = waterTintColor;
    float3 reflectColor = waterTintColor;
    
    if (reflectWater == 1.0f)
    {
        reflectionTexCoord += distortion;
        reflectionTexCoord = clamp(reflectionTexCoord, 0.001f, 0.999f);
        reflectColor = reflectionTexture.Sample(simpleSampler, reflectionTexCoord).rgb;
    }    
    
    if (refractWater == 1.0f)
    {
        refractionTexCoord += distortion;
        refractionTexCoord.x = clamp(refractionTexCoord.x, 0.001f, 0.999f);
        refractionTexCoord.y = clamp(refractionTexCoord.y, -0.999f, -0.001f);
        float depthLevel = saturate((distance(input.worldPos.y, eyePos.y)) / scaling * 5.0f);
        refractColor = lerp(refractionTexture.Sample(simpleSampler, refractionTexCoord), waterTintColor, depthLevel).rgb;
    }
    
    float4 combinedWaterColor = float4(lerp(reflectColor, refractColor, refractiveFactor), 1.0f);
    float4 finalWaterColor = lerp(combinedWaterColor, waterTintColor, 0.2f);
    float faceInLerp = saturate((distance(input.worldPos, eyePos) - faceInStart) / faceInRange);
    return lerp(finalWaterColor, faceInColor, faceInLerp);
}
