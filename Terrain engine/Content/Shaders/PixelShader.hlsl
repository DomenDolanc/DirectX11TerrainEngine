Texture2D terrainTexture;
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

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color;
    if (drawTerrain == 1.0f && useTexture == 1.0f)
    {
        float2 tex = input.worldPos.xz / scaling + 0.5f;
        color = terrainTexture.Sample(simpleSampler, tex);
    } else
        color = float4(input.color, 1.0);
    if (renderShadows == 0.0)
        return color;
	
    const float3 vTol = (lightPos - input.worldPos) / scaling;
    const float distTol = length(vTol);
    const float3 dirTol = vTol / distTol;
	
    const float att = 1.0f / (attConst + attLin * distTol + attQuad * (distTol * distTol));
	
    const float3 diffuse = diffueColor * diffueIntensity * att * max(0.0f, dot(dirTol, input.normal));
    return min(color, float4(saturate((diffuse + ambient)), 1.0f) * color);
}
