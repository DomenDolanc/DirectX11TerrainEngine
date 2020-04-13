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
    };

    struct DrawParamsConstantBuffer
    {
        DirectX::XMFLOAT3 lightPos;
        float scaling;
        DirectX::XMFLOAT3 eyePos;
        float renderShadows;
        TessellationParams tessellationParams;
        DirectX::XMFLOAT2 gridSize;
        DirectX::XMFLOAT2 textureSize;
        float drawTerrain;
        float padding;
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