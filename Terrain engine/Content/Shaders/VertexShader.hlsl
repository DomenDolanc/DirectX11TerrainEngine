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

static const float stepX = 1.0f / textureSize.x;
static const float stepY = 1.0f / textureSize.y;

static const float4 clipPlane = float4(0.0f, 0.0f, 0.0f, 0.0f);

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    float2 outTex = float2((pos.x / scaling) + 0.5, (input.pos.z / scaling) + 0.5);
    float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
    outTex = clamp(outTex, 0.005f, 0.995f);
    pos.y = (sampledTexture.r - 0.1f) * amplitude;
    
    if (usesTessallation == 1.0f)
    {
        output.worldPos = pos.xyz;
        output.normal = input.normal;
        output.color = float3(1.0f, 1.0f, 1.0f);
    } else
    {
        output.color = input.color;

        float zb = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, -stepY), 0).r * amplitude;
        float zc = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(stepX, 0), 0).r * amplitude;
        float zd = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, stepY), 0).r * amplitude;
        float ze = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(-stepX, 0), 0).r * amplitude;
    
        output.normal = normalize(float3(ze - zc, 2.0f, zb - zd));
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
