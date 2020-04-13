cbuffer DrawParamsConstantBuffer : register(b1)
{
    float3 lightPos;
    float scaling;
    float3 eyePos;
    float renderShadows;
    float usesTessallation;
    float drawLOD;
    float2 textureSize;
    float columns;
    float rows;
    float amplitude;
    float drawTerrain;
    float useTexture;
    float3 padding;
};