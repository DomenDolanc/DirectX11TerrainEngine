Texture2D dirtTexture: register(t0);
Texture2D rockTexture: register(t1);
Texture2D grassTexture : register(t2);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"
#include "IncludeFogParams.hlsli"

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float clip : SV_ClipDistance0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

static const float3 ambient = float3(0.202f, 0.233f, 0.292f);

static const float4 waterTintColor = float4(0.0f, 0.6f, 0.8f, 1.0f);

static const float tilingFactor = scaling / 10000.f;

float4 slope_based_color(float slope, float4 colorSteep, float4 colorFlat)
{
    if (slope < 0.25f)
    {
        return colorFlat;
    }
 
    if (slope < 0.5f)
    {
        float blend = (slope - 0.25f) * (1.0f / (0.5f - 0.25f));
 
        return lerp(colorFlat, colorSteep, blend);
    }
    else
    {
        return colorSteep;
    }
}

float4 height_and_slope_based_color(float3 pos, float slope)
{
    float height = pos.y;
    
    float2 tex = pos.xz / scaling + 0.5f;
    float distanceFromCamera = distance(pos, eyePos);
    float mipmapLevel = saturate((distanceFromCamera - 5000.0f) / fogStart) * 6;
    
    float4 dirt = dirtTexture.SampleLevel(simpleSampler, tex * tilingFactor, mipmapLevel);
    float4 rock = rockTexture.SampleLevel(simpleSampler, tex * tilingFactor, mipmapLevel);
    float4 grass = grassTexture.SampleLevel(simpleSampler, tex * tilingFactor, mipmapLevel);
    float4 snow = float4(1.0f, 1.0f, 1.0f, 1.0f);
 
    float greenBlendEnd = amplitude * 0.35f;
    float greenBlendStart = amplitude * 0.1f; 
    float snowBlendEnd = amplitude * 0.6f;
 
    if (height < greenBlendStart)
    {
        // get grass/dirt values
        return slope_based_color(slope, dirt, grass);
    }
 
    if (height < greenBlendEnd)
    {
        // get both grass/dirt values and rock values and blend
        float4 c1 = slope_based_color(slope, dirt, grass);
        float4 c2 = rock;
     
        float blend = (height - greenBlendStart) * (1.0f / (greenBlendEnd - greenBlendStart));
         
        return lerp(c1, c2, blend);
    }
 
    if (height < snowBlendEnd)
    {
        // get rock values and rock/snow values and blend
        float4 c1 = rock;
        float4 c2 = slope_based_color(slope, rock, snow);
         
        float blend = (height - greenBlendEnd) * (1.0f / (snowBlendEnd - greenBlendEnd));
 
        return lerp(c1, c2, blend);
    }
 
    // get rock/snow values
    return slope_based_color(slope, rock, snow);
}

float4 main(PixelShaderInput input) : SV_TARGET
{    
    float4 color;
    if (useTexture == 1.0f)
    {
        float slope = acos(clamp(input.normal.y, input.normal.y, 0.999f));
        color = height_and_slope_based_color(input.worldPos, slope);
    } 
    else
        color = float4(input.color, 1.0);
    
    float3 rayDir = normalize(input.worldPos - eyePos);
    
    float fogAmount = saturate(1.0 - exp(-distance(input.worldPos, eyePos) * fogDensity));
    float sunAmount = max(dot(rayDir, normalize(lightPos)), 0.0);
    float3 fogColorMixed = lerp(blueFogColor, yellowFogColor, pow(sunAmount, sunAmountFactor));
    color = lerp(color, float4(fogColorMixed, 1.0f), fogAmount);
    
    float faceInLerp = saturate((distance(input.worldPos, eyePos) - faceInStart) / faceInRange);
    color = lerp(color, faceInColor, faceInLerp);
    
    if (renderShadows == 0.0)
        return color;
    
    float diffuse = saturate(dot(input.normal, normalize(lightPos))); 
    float4 shadedColor = float4(saturate((color.rgb * diffuse) + (color.rgb * ambient)), color.a);
    
    if (eyePos.y >= 0.0f)
        return shadedColor;
    else    
        return lerp(waterTintColor, shadedColor, 0.5f);
}
