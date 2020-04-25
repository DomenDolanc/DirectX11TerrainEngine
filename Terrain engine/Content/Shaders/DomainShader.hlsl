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

static const float stepX = 1.0f / textureSize.x;
static const float stepY = 1.0f / textureSize.y;

#define NUM_CONTROL_POINTS 4

[domain("quad")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float2 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    DS_OUTPUT Output;

    Output.pos = lerp(lerp(patch[0].pos, patch[1].pos, domain.x), lerp(patch[2].pos, patch[3].pos, domain.x), domain.y);
    //Output.clip = lerp(lerp(patch[0].clip, patch[1].clip, domain.x), lerp(patch[2].clip, patch[3].clip, domain.x), domain.y);
	
    float2 outTex = float2((Output.pos.x / scaling) + 0.5, (Output.pos.z / scaling) + 0.5);
    float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0).rgb;
    
    float zb = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, -stepY), 0).r * amplitude;
    float zc = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(stepX, 0), 0).r * amplitude;
    float zd = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, stepY), 0).r * amplitude;
    float ze = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(-stepX, 0), 0).r * amplitude;
    
    Output.normal = normalize(float3(ze - zc, 2.0f, zb - zd));
    Output.pos.y = (sampledTexture.r - 0.1f) * amplitude;
    Output.worldPos = mul(Output.pos, model).xyz;
    if (drawTerrain == 0.0f || clipForReflection == 0.0f)
        Output.clip = 1.0f;
    else if (clipForReflection == 1.0f && Output.worldPos.y >= 0.0f)
        Output.clip = 1.0f;
    else if (clipForReflection == -1.0f && Output.worldPos.y <= 0.0f)
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
