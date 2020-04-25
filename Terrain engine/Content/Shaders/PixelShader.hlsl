Texture2D dirtTexture: register(t0);
Texture2D rockTexture: register(t1);
Texture2D grassTexture : register(t2);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

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
static const float4 fogColor = float4(0.9f, 0.9f, 0.9f, 0.0f);
static const float fogStart = 23000.0f;
static const float fogRange = 8000.0f;

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
    
    float2 tex = pos.xz / scaling;
    float4 dirt = dirtTexture.Sample(simpleSampler, tex * 10 + 0.5f);
    float4 rock = rockTexture.Sample(simpleSampler, tex * 10 + 0.5f);
    float4 grass = grassTexture.Sample(simpleSampler, tex * 10 + 0.5f);
    float4 snow = float4(0.8f, 0.8f, 0.8f, 1.0f);
 
    float bounds = amplitude * 0.02f;
    float transition = amplitude * 0.3f;
    float greenBlendEnd = transition + bounds;
    float greenBlendStart = transition - bounds;
    float snowBlendEnd = greenBlendEnd + 2 * bounds;
 
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
    if (drawTerrain == 1.0f && useTexture == 1.0f)
    {
        float slope = acos(input.normal.y);
        color = height_and_slope_based_color(input.worldPos, slope);

    } 
    else
        color = float4(input.color, 1.0);
    
    if (drawTerrain == 1.0f)
    {
        float fogLerp = saturate((distance(input.worldPos, eyePos) - fogStart) / fogRange);
        color = lerp(color, fogColor, fogLerp);
    }
    if (renderShadows == 0.0)
        return color;
	
    const float3 vTol = (lightPos - input.worldPos) / scaling;
    const float distTol = length(vTol);
    const float3 dirTol = vTol / distTol;
	
    const float att = 1.0f / (attConst + attLin * distTol + attQuad * (distTol * distTol));
	
    const float3 diffuse = diffueColor * diffueIntensity * att * max(0.0f, dot(dirTol, input.normal));
    return min(color, float4(saturate((diffuse + ambient)), 1.0f) * color);
}
