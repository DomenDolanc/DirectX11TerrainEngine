#pragma once

namespace Terrain_engine
{
    struct ModelViewProjectionConstantBuffer
    {
        DirectX::XMFLOAT4X4 mvp;
    };

    struct TessellationParams
    {
        float usesTessellation;
        float drawLOD;
        float useCulling;
    };

    struct TerrainParams
    {
        float columns;
        float rows;
        float amplitude;
        float tiling;
    };

    struct WaterParamsConstantBuffer
    {
        float reflectWater;
        float refractWater;
        float waterMoveFactor;
        float waterStrengthFactor;
    };

    struct DrawParamsConstantBuffer
    {
        DirectX::XMFLOAT3 lightPos;
        float scaling;

        DirectX::XMFLOAT3 eyePos;
        float renderShadows;

        TerrainParams terrainParams;

        float useTexture;
        TessellationParams tessellationParams;
        
        DirectX::XMFLOAT2 textureSize;
        float clipPlaneType;
        float drawTerrain;
        
        DirectX::XMFLOAT4 planes[6];
    };

    struct VertexPosition
    {
        DirectX::XMFLOAT3 pos;
    };

    struct VertexData
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 texCoord;
    };
}