#pragma once

namespace Terrain_engine
{
    struct ModelViewProjectionConstantBuffer
    {
        DirectX::XMFLOAT4X4 model;
        DirectX::XMFLOAT4X4 view;
        DirectX::XMFLOAT4X4 projection;
    };

    struct DrawParamsConstantBuffer
    {
        DirectX::XMFLOAT3 lightPos;
        float scaling;
        float renderShadows;
        float padding2;
        float padding3;
        float padding4;
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