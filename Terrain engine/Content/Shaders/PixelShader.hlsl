Texture2D dirtTexture: register(t0);
Texture2D rockTexture: register(t1);
//Texture2D snowTexture: register(t2);
SamplerState simpleSampler;

#include "IncludeDrawParams.hlsli"

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

static const float3 materialColor = float3(0.7f, 0.7f, 0.9f);
static const float3 ambient = float3(0.4f, 0.4f, 0.4f);
static const float3 diffueColor = float3(1.0f, 1.0f, 1.0f);
static const float diffueIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.55f;
static const float attQuad = 0.95f;
static const float4 fogColor = float4(0.9f, 0.9f, 0.9f, 0.0f);
static const float fogStart = 15000.0f;
static const float fogRange = 4000.0f;

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color;
    if (drawTerrain == 1.0f && useTexture == 1.0f)
    {
        float2 tex = input.worldPos.xz / scaling * 10 + 0.5f;
        float4 dirtColor = dirtTexture.Sample(simpleSampler, tex);
        float4 rockColor = rockTexture.Sample(simpleSampler, tex);
        //float4 snowColor = snowTexture.Sample(simpleSampler, tex);
        float slope = 1.0f - input.normal.y;
        
        if (slope < 0.2f)
        {
            float blendAmount = slope / 0.2f;
            color = lerp(dirtColor, rockColor, blendAmount);
        }
        else
        {
            float blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
            color = lerp(dirtColor, rockColor, blendAmount);
        }

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
