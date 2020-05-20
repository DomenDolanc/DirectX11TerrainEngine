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
    float clip : SV_ClipDistance0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

static const float tilingFactor = 5.0f;

static const float4 clipPlane = float4(0.0f, 0.0f, 0.0f, 0.0f);

inline float3 calculateNormal(float3 worldPos)
{
    float patchColumnStep = scaling / (columns - 1);
    float patchRowStep = scaling / (rows - 1);
    
    float2 top = float2(worldPos.x, worldPos.z - patchRowStep) / scaling + 0.5f;
    float2 right = float2(worldPos.x + patchColumnStep, worldPos.z) / scaling + 0.5f;
    float2 bottom = float2(worldPos.x, worldPos.z + patchRowStep) / scaling + 0.5f;
    float2 left = float2(worldPos.x - patchColumnStep, worldPos.z) / scaling + 0.5f;
    
    float zb = (heightMapTexture.SampleLevel(simpleSampler, top * tilingFactor, 0).r - 0.1f) * amplitude;
    float zc = (heightMapTexture.SampleLevel(simpleSampler, right * tilingFactor, 0).r - 0.1f) * amplitude;
    float zd = (heightMapTexture.SampleLevel(simpleSampler, bottom * tilingFactor, 0).r - 0.1f) * amplitude;
    float ze = (heightMapTexture.SampleLevel(simpleSampler, left * tilingFactor, 0).r - 0.1f) * amplitude;
    
    float3 tangent = float3(0.0f, (zb - zd), -2.0f * patchRowStep);
    float3 bitan = float3(-2.0f * patchColumnStep, (ze - zc), 0.0f);
    return normalize(cross(tangent, bitan));
}

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    float2 outTex = clamp((pos.xz / scaling + 0.5f), 0.005f, 0.995f) * tilingFactor;
    float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
    pos.y = (sampledTexture.r - 0.1f) * amplitude;
    
    if (usesTessallation == 1.0f)
    {
        output.worldPos = pos.xyz;
        output.normal = input.normal;
        output.color = float3(1.0f, 1.0f, 1.0f);
    } else
    {
        output.color = input.color;    
        output.normal = calculateNormal(pos.xyz);
        output.color = sampledTexture;
        output.worldPos = pos.xyz;

        pos = mul(pos, model);
        pos = mul(pos, view);
        pos = mul(pos, projection);
    }
    
    if (clipPlaneType == 0.0f)
        output.clip = 1.0f;
    else if (clipPlaneType == 1.0f && output.worldPos.y >= 0.0f)
        output.clip = 1.0f;
    else if (clipPlaneType == -1.0f && output.worldPos.y <= 0.0f)
        output.clip = 1.0f;
    else
        output.clip = -1.0f;
    output.pos = pos;
    
    return output;
}
