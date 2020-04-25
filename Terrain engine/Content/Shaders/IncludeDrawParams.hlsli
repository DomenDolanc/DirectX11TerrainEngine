cbuffer DrawParamsConstantBuffer : register(b1)
{
    float3 lightPos;
    float scaling;
    
    float3 eyePos;
    float renderShadows;
    
    float columns;
    float rows;
    float amplitude;
    float drawTerrain;
    
    float useTexture;
    float usesTessallation;
    float drawLOD;
    float useCulling;
    
    float2 textureSize;
    float clipPlaneType;
    float waterMoveFactor;
    
    float4 frustum[6];
};