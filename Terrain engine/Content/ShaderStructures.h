#pragma once

namespace Terrain_engine
{
    struct ModelViewProjectionConstantBuffer
    {
        DirectX::XMFLOAT4X4 model;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
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
    };

    struct DrawParamsConstantBuffer
    {
        DirectX::XMFLOAT3 lightPos;
        float scaling;
        DirectX::XMFLOAT3 eyePos;
        float renderShadows;
        TerrainParams terrainParams;
        float drawTerrain;
        float useTexture;
        TessellationParams tessellationParams;
        DirectX::XMFLOAT2 textureSize;
        DirectX::XMFLOAT2 padding;
        DirectX::XMFLOAT4 planes[6];
    };

    struct VertexPositionColor
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 color;
        DirectX::XMFLOAT3 normal;
    };

    struct VertexPosition
    {
        DirectX::XMFLOAT3 pos;
        DirectX::XMFLOAT3 normal;
    };
}