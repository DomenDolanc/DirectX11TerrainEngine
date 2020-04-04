cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer DrawParamsConstantBuffer : register(b1)
{
    float scaling;
    float renderShadows;
    float padding2;
    float padding3;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
    float3 normal: NORMAL0;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal: NORMAL0;
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
    output.normal = input.normal;

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
    output.color = input.color;

	return output;
}
