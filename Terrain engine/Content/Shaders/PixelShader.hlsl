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

static const float3 materialColor = float3(0.7f, 0.7f, 0.9f);
static const float3 ambient = float3(0.4f, 0.4f, 0.4f);
static const float3 diffueColor = float3(1.0f, 1.0f, 1.0f);
static const float diffueIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.55f;
static const float attQuad = 0.95f;

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
    
    float fogLerp = 1.0 - exp(-distance(input.worldPos, eyePos) / scaling * 8 * color.b);
    float faceInLerp = saturate((distance(input.worldPos, eyePos) - faceInStart) / faceInRange);
    
    color = lerp(color, fogColor, fogLerp);
    color = lerp(color, faceInColor, faceInLerp);
    
    if (renderShadows == 0.0)
        return color;
	
    const float3 vTol = (lightPos - input.worldPos) / scaling;
    const float distTol = length(vTol);
    const float3 dirTol = vTol / distTol;
	
    const float att = 1.0f / (attConst + attLin * distTol + attQuad * (distTol * distTol));
	
    const float3 diffuse = diffueColor * diffueIntensity * att * max(0.0f, dot(dirTol, input.normal));
    
    float4 shadedColor = min(color, float4(saturate((diffuse + ambient)), 1.0f) * color);
    if (eyePos.y >= 0.0f)
        return shadedColor;
    else    
        return lerp(waterTintColor, shadedColor, 0.5f);
}
