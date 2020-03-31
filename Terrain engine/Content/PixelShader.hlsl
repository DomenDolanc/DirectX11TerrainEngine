Texture2D shadowMap;
SamplerState simpleSampler;

cbuffer DrawParamsConstantBuffer : register(b1)
{
    float scaling;
    float renderShadows;
    float padding2;
    float padding3;
};

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR0;
    float3 normal: NORMAL0;
};

static float4 ambientVector = float4(0.80, 0.80, 0.80, 1);
static float3 lightVector = float3(-1, -0.5, -0.5);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = float4(input.color, 1.0);
    if (renderShadows == 0.0)
        return color;

    float4 shadedColor;
    float3 lightIntensity = saturate(dot(input.normal, -lightVector));
    shadedColor = color * float4(lightIntensity, 1.0f) + ambientVector * color;
    shadedColor.w = 1.0;
    return min(color, shadedColor);
}
