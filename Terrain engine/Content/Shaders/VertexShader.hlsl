Texture2D heightMapTexture;
SamplerState simpleSampler;

cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix model;
    matrix view;
    matrix projection;
};

cbuffer DrawParamsConstantBuffer : register(b1)
{
    float3 lightPos;
    float scaling;
    float3 eyePos;
    float renderShadows;
    float usesTessallation;
    float drawLOD;
    float2 gridSize;
    float2 textureSize;
    float drawTerrain;
    float padding;
};

struct VertexShaderInput
{
    float3 pos : POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
};

struct GeometryShaderInput
{
    float4 pos : SV_POSITION;
    float3 color : COLOR0;
    float3 normal : NORMAL0;
    float3 worldPos : POSITIONT;
};

static const float stepX = 1.0f / textureSize.x;
static const float stepY = 1.0f / textureSize.y;

GeometryShaderInput main(VertexShaderInput input)
{
    GeometryShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);

    if (usesTessallation == 1.0f)
    {
        output.worldPos = pos;
        output.normal = input.normal;
        output.color = float3(1.0f, 1.0f, 1.0f);
    } else
    {
        output.worldPos = mul(pos, model);
        output.color = input.color;
        if (drawTerrain)
        {
            float2 outTex = float2((pos.x / scaling) + 0.5, (input.pos.z / scaling) + 0.5);
    
            const float heightScale = scaling / 8.0f;
            float3 sampledTexture = heightMapTexture.SampleLevel(simpleSampler, outTex, 0);
    
            float zb = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, -stepY), 0).r * heightScale;
            float zc = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(stepX, 0), 0).r * heightScale;
            float zd = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(0, stepY), 0).r * heightScale;
            float ze = heightMapTexture.SampleLevel(simpleSampler, outTex + float2(-stepX, 0), 0).r * heightScale;
    
            output.normal = normalize(float3(ze - zc, 2.0f, zb - zd));
            pos.y = sampledTexture.r * heightScale;
            output.color = sampledTexture;
        }

        pos = mul(pos, model);  
        pos = mul(pos, view);
        pos = mul(pos, projection);
    }
    output.pos = pos;

    return output;
}
