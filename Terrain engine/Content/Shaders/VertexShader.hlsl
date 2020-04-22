Texture2D heightMapTexture;
SamplerState simpleSampler;

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

#include "IncludeDrawParams.hlsli"

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
    float clip : SV_ClipDistance0;
};

static const float stepX = 1.0f / textureSize.x;
static const float stepY = 1.0f / textureSize.y;

static const float4 clipPlane = float4(0.0f, -500.0f, 0.0f, 0.0f);

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    
    output.normal = input.normal;
    output.color = input.color;
    output.worldPos = pos.xyz;
    
    if (usesTessallation == 1.0f)
    {
        float2 outTex = float2((pos.x / scaling) + 0.5, (input.pos.z / scaling) + 0.5);
        float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
        pos.y = (1.1f * sampledTexture.r - 0.1f) * amplitude;
        output.worldPos = pos.xyz;
        output.normal = input.normal;
        output.color = float3(1.0f, 1.0f, 1.0f);
    } else
    {
        output.color = input.color;
        if (drawTerrain)
        {
            float2 outTex = float2((pos.x / scaling) + 0.5, (input.pos.z / scaling) + 0.5);
            float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
            float zb = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, -stepY), 0).r * amplitude;
            float zc = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(stepX, 0), 0).r * amplitude;
            float zd = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, stepY), 0).r * amplitude;
            float ze = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(-stepX, 0), 0).r * amplitude;
    
            output.normal = normalize(float3(ze - zc, 2.0f, zb - zd));
            pos.y = (1.1f * sampledTexture.r - 0.1f) * amplitude;
            output.color = sampledTexture;
        }
        output.worldPos = pos.xyz;

        pos = mul(pos, model);  
        pos = mul(pos, view);
        pos = mul(pos, projection);
    }
    
    output.clip = (!drawTerrain || output.worldPos.y >= 0.0f ? 1.0f : -1.0f);
    output.pos = pos;
    
    return output;
}
