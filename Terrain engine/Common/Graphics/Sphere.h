#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DirectXMesh.h"

namespace Terrain_engine
{
    class Sphere
    {
    public:
        Sphere(std::shared_ptr<DX::DeviceResources> deviceResources);
        Sphere(std::shared_ptr<DX::DeviceResources> deviceResources, DirectX::XMFLOAT3 pos, int radius);
        ~Sphere();

        void CreateVertices();
        void CreateIndices();

        void setScaling(double scaling);

        void ResetBuffers();

        void Draw();
    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
        double m_radius = 1000.0f;
        const int m_sliceCount = 50;
        const int m_stackCount = 50;

        std::vector<VertexPositionColor> m_vertices;
        std::vector<uint32_t> m_indices;
        size_t	m_verticesCount;
        size_t	m_indexCount;

        double m_scaling;

        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}