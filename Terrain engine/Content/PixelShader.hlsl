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
    float4 lightViewPos: TEXCOORD0;
};

static float4 ambientVector = float4(0.80, 0.80, 0.80, 1);
static float3 lightVector = float3(-1, -0.5, -0.5);

float4 main(PixelShaderInput input) : SV_TARGET
{
    float4 color = float4(input.color, 1.0);
    if (renderShadows == 0.0)
        return color;

    //float4 shadedColor;
    //float3 lightIntensity = saturate(dot(input.normal, -lightVector));
    //shadedColor = color * float4(lightIntensity, 1.0f) + ambientVector * color;
    //shadedColor.w = 1.0;
    //return min(color, shadedColor);

    input.lightViewPos.xyz /= input.lightViewPos.w;

    //if position is not visible to the light - dont illuminate it
    //results in hard light frustum
    if (input.lightViewPos.x < -1.0f || input.lightViewPos.x > 1.0f ||
        input.lightViewPos.y < -1.0f || input.lightViewPos.y > 1.0f ||
        input.lightViewPos.z < 0.0f || input.lightViewPos.z > 1.0f)
        return float4(1.0, 0.0, 0.0, 1.0); // ambientVector* color;

    //transform clip space coords to texture space coords (-1:1 to 0:1)
    input.lightViewPos.x = input.lightViewPos.x / 2 + 0.5;
    input.lightViewPos.y = input.lightViewPos.y / -2 + 0.5;

    //sample shadow map - point sampler
    float shadowMapDepth = shadowMap.Sample(simpleSampler, input.lightViewPos.xy).r;
    
    if (shadowMapDepth + 0.000004 > input.lightViewPos.z)
    {
        return color;
        //float4 shadedColor;
        //float3 lightIntensity = saturate(dot(input.normal, -lightVector));
        //shadedColor = color * float4(lightIntensity, 1.0f) + ambientVector * color;
        //shadedColor.w = 1.0;
        //return min(color, shadedColor);
    }
    else
        return ambientVector* color;
}
