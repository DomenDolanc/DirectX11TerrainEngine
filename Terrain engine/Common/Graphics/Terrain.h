#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\PerlinNoise.h"
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

        size_t getVerticesCount();
        size_t getIndicesCount();

        void setScaling(double scaling);

        void setMode(int index);

        void ResetBuffers();

        void Draw();

        std::shared_ptr<PerlinNoise> getPerlinNoise();

        DirectX::XMFLOAT3 GetColorFromHeight(double height);

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        int m_Columns = 64;
        int m_Rows = 64;
        double m_scaling;

        std::vector<VertexPositionColor> m_vertices;
        std::vector<uint32_t> m_indices;
        size_t	m_verticesCount;
        size_t	m_indexCount;
        
        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

        static std::map<int, DirectX::XMFLOAT3> m_TerrainColormap;

        std::shared_ptr<PerlinNoise> m_PerlinNoise;
    };
}

