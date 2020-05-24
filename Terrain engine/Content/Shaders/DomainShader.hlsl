Texture2D heightMapTexture;
SamplerState simpleSampler;

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

#include "IncludeDrawParams.hlsli"

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float clip : SV_ClipDistance0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float clip : SV_ClipDistance0;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITION0;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
    float EdgeTessFactor[4] : SV_TessFactor; // e.g. would be [4] for a quad domain
    float InsideTessFactor[2] : SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

inline float3 calculateNormal(float3 worldPos, float tessellationFactor)
{
    float patchColumnStep = scaling / (columns - 1) / tessellationFactor;
    float patchRowStep = scaling / (rows - 1) / tessellationFactor;
    
    float2 top = float2(worldPos.x, worldPos.z - patchRowStep) / scaling + 0.5f;
    float2 right = float2(worldPos.x + patchColumnStep, worldPos.z) / scaling + 0.5f;
    float2 bottom = float2(worldPos.x, worldPos.z + patchRowStep) / scaling + 0.5f;
    float2 left = float2(worldPos.x - patchColumnStep, worldPos.z) / scaling + 0.5f;
    
    float zb = (heightMapTexture.SampleLevel(simpleSampler, top * terrainTiling, 0).r - 0.1f) * amplitude;
    float zc = (heightMapTexture.SampleLevel(simpleSampler, right * terrainTiling, 0).r - 0.1f) * amplitude;
    float zd = (heightMapTexture.SampleLevel(simpleSampler, bottom * terrainTiling, 0).r - 0.1f) * amplitude;
    float ze = (heightMapTexture.SampleLevel(simpleSampler, left * terrainTiling, 0).r - 0.1f) * amplitude;
    
    float3 tangent = float3(0.0f, (zb - zd), -2.0f * patchRowStep);
    float3 bitan = float3(-2.0f * patchColumnStep, (ze - zc), 0.0f);
    return normalize(cross(tangent, bitan));
}

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;

    Output.pos = lerp(lerp(patch[0].pos, patch[1].pos, domain.x), lerp(patch[2].pos, patch[3].pos, domain.x), domain.y);
	
    float2 outTex = clamp((Output.pos.xz / scaling + 0.5f), 0.005f, 0.995f) * terrainTiling;
    float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
    
    Output.normal = calculateNormal(Output.pos.xyz, input.InsideTessFactor[0]);
    Output.pos.y = (sampledTexture.r - 0.1f) * amplitude;
    Output.worldPos = mul(Output.pos, model).xyz;
    if (clipPlaneType == 0.0f)
        Output.clip = 1.0f;
    else if (clipPlaneType == 1.0f && Output.worldPos.y >= 0.0f)
        Output.clip = 1.0f;
    else if (clipPlaneType == -1.0f && Output.worldPos.y <= 0.0f)
        Output.clip = 1.0f;
    else
        Output.clip = -1.0f;
    Output.pos = mul(Output.pos, model);
    Output.pos = mul(Output.pos, view);
    Output.pos = mul(Output.pos, projection);
    if (drawLOD)
        Output.color = lerp(lerp(patch[0].color, patch[1].color, domain.x), lerp(patch[2].color, patch[3].color, domain.x), domain.y);
    else
        Output.color = sampledTexture;

    return Output;
}
