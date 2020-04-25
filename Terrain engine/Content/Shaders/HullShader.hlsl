#include "IncludeDrawParams.hlsli"

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

// Input control point
struct VS_CONTROL_POINT_OUTPUT
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
	float EdgeTessFactor[4]			: SV_TessFactor; // e.g. would be [4] for a quad domain
	float InsideTessFactor[2]			: SV_InsideTessFactor; // e.g. would be Inside[2] for a quad domain
	// TODO: change/add other stuff
};

#define NUM_CONTROL_POINTS 4

static const float minLODDistance = 0.0025f * scaling;
static const float maxLODDistance = 0.3f * scaling;
static const float maxTessFactorX = min(6.0f, trunc(12.0f - log2(columns)));
static const float maxTessFactorY = min(6.0f, trunc(12.0f - log2(rows)));

static const float3 colorLOD[6] = {
    float3(1.0f, 0.0f, 0.0f),       
    float3(1.0f, 1.0f, 0.0f),       
    float3(0.0f, 1.0f, 0.0f),       
    float3(0.0f, 1.0f, 1.0f),       
    float3(0.0f, 0.0f, 1.0f),       
    float3(1.0f, 0.0f, 1.0f),       
};

float CalcTessFactor(float3 p)
{
    float d = distance(p, eyePos);
 
    float s = saturate((d - minLODDistance) / (maxLODDistance - minLODDistance));
    return pow(2, (lerp(min(maxTessFactorX, maxTessFactorY), 0, s)));
}

// returns true if the box is completely behind the plane.
bool aabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
    float4 pl = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 n = abs(plane.xyz); // |n.x|, |n.y|, |n.z|
    float e = dot(extents, n); // always positive
    float s = dot(float4(center, 1.0f), plane); // signed distance from center point to plane
    
    // if the center point of the box is a distance of e or more behind the plane
    // (in which case s is negative since it is behind the plane), then the box
    // is completely in the negative half space of the plane.
    return (s + e) < 0.0f;
}

// returns true if the box is completely outside the frustum.
bool aabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6])
{
    // if the box is completely behind any of the frustum planes, then it is outside the frustum.
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[0]))
        return true;
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[1]))
        return true;
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[2]))
        return true;
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[3]))
        return true;
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[4]))
        return true;
    if (aabbBehindPlaneTest(center, extents, frustumPlanes[5]))
        return true;
    return false;
}

bool isPatchCulled(InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    float4 pos0 = patch[0].pos / scaling * 2;
    pos0.y = -1.0f;
    float4 pos3 = patch[3].pos / scaling * 2;
    pos3.y = 1.0f;

    
    pos0 = mul(pos0, model);
    pos0 = mul(pos0, view);
    pos0 = mul(pos0, projection);
    
    pos0.x *= -1.0f;
    pos0.z *= -1.0f;
    
    pos3 = mul(pos3, model);
    pos3 = mul(pos3, view);
    pos3 = mul(pos3, projection);
    pos3.x *= -1.0f;
    pos3.z *= -1.0f;
    
    float3 vMin = (float3(pos0.x, pos0.y, pos0.z));
    float3 vMax = (float3(pos3.x, pos3.y, pos3.z));
     
    // center/extents representation.
    float3 boxCenter = 0.5f * (vMin + vMax);
    float3 boxExtents = 0.5f * (vMax - vMin);
 
    return aabbOutsideFrustumTest(boxCenter, boxExtents, frustum);
}
 
// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint PatchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT Output;
    
    if (useCulling && isPatchCulled(ip))
    {
        Output.EdgeTessFactor[0] = 0.0f;
        Output.EdgeTessFactor[1] = 0.0f;
        Output.EdgeTessFactor[2] = 0.0f;
        Output.EdgeTessFactor[3] = 9.0f;
        Output.InsideTessFactor[0] = 0.0f;
        Output.InsideTessFactor[1] = 0.0f;
    }
    else
    {
        float3 e0 = 0.5f * (ip[0].pos + ip[2].pos).xyz;
        float3 e1 = 0.5f * (ip[0].pos + ip[1].pos).xyz;
        float3 e2 = 0.5f * (ip[1].pos + ip[3].pos).xyz;
        float3 e3 = 0.5f * (ip[2].pos + ip[3].pos).xyz;
        float3 c = 0.25f * (ip[0].pos + ip[1].pos + ip[2].pos + ip[3].pos).xyz;
 
        Output.EdgeTessFactor[0] = CalcTessFactor(e0);
        Output.EdgeTessFactor[1] = CalcTessFactor(e1);
        Output.EdgeTessFactor[2] = CalcTessFactor(e2);
        Output.EdgeTessFactor[3] = CalcTessFactor(e3);
        Output.InsideTessFactor[0] = CalcTessFactor(c);
        Output.InsideTessFactor[1] = Output.InsideTessFactor[0];
    }

	return Output;
}

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
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
    
    if (drawLOD)
        Output.color = colorLOD[min(CalcTessFactor(0.25f * (ip[0].pos + ip[1].pos + ip[2].pos + ip[3].pos).xyz), 5)];
    else
        Output.color = ip[i].color;
    Output.clip = ip[i].clip;

	return Output;
}
