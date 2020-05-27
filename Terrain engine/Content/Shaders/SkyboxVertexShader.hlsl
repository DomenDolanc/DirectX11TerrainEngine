cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
    matrix mvp;
};

struct VertexShaderInput
{
    float3 pos : POSITION;
};

struct PixelShaderInput
{
    float4 pos : SV_POSITION;
    float3 texCoord : TEXCOORD0;
};


PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    float4 pos = float4(input.pos, 1.0f);
    
    output.texCoord = input.pos;
    
    pos = mul(pos, mvp);
    output.pos = pos;

    return output;
}