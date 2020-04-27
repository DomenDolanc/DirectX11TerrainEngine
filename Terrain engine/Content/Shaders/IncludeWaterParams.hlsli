cbuffer WaterParamsConstantBuffer : register(b1)
{
    float scaling;
    float3 eyePos;
    
    float reflectWater;
    float refractWater;
    float waterMoveFactor;
    float waterStrengthFactor;
};