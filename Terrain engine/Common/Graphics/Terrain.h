#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\Graphics\Camera.h"
#include "..\Common\DirectXMesh.h"
#include <map>

namespace Terrain_engine
{
    class Terrain
    {
    public:
        Terrain(std::shared_ptr<DX::DeviceResources> deviceResources);
        Terrain(std::shared_ptr<DX::DeviceResources> deviceResources, int columns, int rows);
        ~Terrain();

        void CreateVertices();
        void CreateIndices();
        void CreateQuadIndices();

        size_t getVerticesCount();
        size_t getIndicesCount();

        void setScaling(double scaling);

        DirectX::XMFLOAT2 getGridSize();

        void ResetBuffers();

        void Draw();

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        int m_Columns = 100;
        int m_Rows = 100;
        double m_scaling;

        std::vector<VertexPositionColor> m_vertices;
        std::vector<uint32_t> m_indices;
        size_t	m_verticesCount;
        size_t	m_indexCount;
        
        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
    };
}

