#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"

namespace Terrain_engine
{
    class Skybox
    {
    public:
        Skybox(std::shared_ptr<DX::DeviceResources> deviceResources);
        ~Skybox();

        void CreateVertices();
        void CreateIndices();

        void setScaling(float scaling);

        void ResetBuffers();

        ID3D11Texture2D* GetSkyboxTexture() const { return m_d3dSkyboxTexture.Get(); }
        ID3D11ShaderResourceView* GetSkyboxTextureShaderResourceView() const { return m_d3dSkyboxTextureShaderView.Get(); }

        void Draw();
    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        std::vector<VertexPosition> m_vertices;
        std::vector<uint32_t> m_indices;
        size_t	m_verticesCount;
        size_t	m_indexCount;

        float m_scaling;

        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_d3dSkyboxTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_d3dSkyboxTextureShaderView;
    };
}