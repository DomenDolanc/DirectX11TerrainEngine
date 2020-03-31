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

cbuffer LightConstantBuffer : register(b2)
{
    matrix lightModel;
    matrix lightView;
    matrix lightProjection;
};

struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 color : COLOR0;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal: NORMAL0;
    float4 lightViewPos: TEXCOORD0;
};

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
    pos.y = pos.y * scaling/20;
    output.normal = pos;

	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
    output.color = input.color;

    float4 lightPos = float4(input.pos, 1.0f);
    lightPos = mul(lightPos, lightView);
    lightPos = mul(lightPos, lightProjection);
    output.lightViewPos = lightPos;

	return output;
}
