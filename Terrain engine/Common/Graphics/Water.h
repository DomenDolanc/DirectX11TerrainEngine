#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DirectXMesh.h"

namespace Terrain_engine
{
    class Water
    {
    public:
        Water(std::shared_ptr<DX::DeviceResources> deviceResources);
        ~Water();

        void CreateVertices();
        void CreateIndices();

        void ResetBuffers();

        void setScaling(float scaling);

        void Draw();
    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };

        float m_scaling = 1.0f;

        std::vector<VertexPositionColor> m_vertices;
        std::vector<uint32_t> m_indices;
        size_t	m_verticesCount;
        size_t	m_indexCount;
        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}