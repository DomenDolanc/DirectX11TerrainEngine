cbuffer DrawParamsConstantBuffer : register(b1)
{
    float3 lightPos;
    float scaling;
    float3 eyePos;
    float renderShadows;
    float3 padding4;
    float usesTessallation;
    float4 tesselationParams;
};

// Input control point
struct VS_CONTROL_POINT_OUTPUT
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

static const float minLODDistance = 0.025f * scaling;
static const float maxLODDistance = 0.3f * scaling;

float CalcTessFactor(float3 p)
{
    float d = distance(p, eyePos);
 
    float s = saturate((d - minLODDistance) / (maxLODDistance - minLODDistance));
    return pow(2, (lerp(6, 0, s)));
}

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;

     // tessellate based on distance from the camera.
    // compute tess factor based on edges.
    // compute midpoint of edges.
    float3 e0 = 0.5f * (ip[0].pos + ip[2].pos);
    float3 e1 = 0.5f * (ip[0].pos + ip[1].pos);
    float3 e2 = 0.5f * (ip[1].pos + ip[2].pos);
    float3 c = 0.25f * (ip[0].pos + ip[1].pos + ip[2].pos);
 
    Output.EdgeTessFactor[0] = CalcTessFactor(e0);
    Output.EdgeTessFactor[1] = CalcTessFactor(e1);
    Output.EdgeTessFactor[2] = CalcTessFactor(e2);
    Output.InsideTessFactor = CalcTessFactor(c);

	return Output;
}

[domain("tri")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint PatchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT Output;

	// Insert code to compute Output here
	Output.pos = ip[i].pos;
	Output.normal = ip[i].normal;
	Output.worldPos = ip[i].worldPos;
	Output.color = ip[i].color;

	return Output;
}
