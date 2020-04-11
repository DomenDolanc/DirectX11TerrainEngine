cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

struct DS_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

// Output control point
struct HS_CONTROL_POINT_OUTPUT
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 3

[domain("tri")]
DS_OUTPUT main(
	HS_CONSTANT_DATA_OUTPUT input,
	float3 domain : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
	DS_OUTPUT Output;

    Output.pos = float4(
		patch[0].pos.xyz * domain.x + patch[1].pos.xyz * domain.y + patch[2].pos.xyz * domain.z, 1);
    Output.normal = float3(
		patch[0].normal * domain.x + patch[1].normal * domain.y + patch[2].normal * domain.z);
    Output.color = float4(
		patch[0].color.xyz * domain.x + patch[1].color.xyz * domain.y + patch[2].color.xyz * domain.z, 1);
	
    Output.worldPos = mul(Output.pos, model);
    Output.pos = mul(Output.pos, model);
    Output.pos = mul(Output.pos, view);
    Output.pos = mul(Output.pos, projection);

	return Output;
}
