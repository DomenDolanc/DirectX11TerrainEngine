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

        void CreateReflectionTexture(Windows::Foundation::Size textureSize);
        void CreateRefrationTexture(Windows::Foundation::Size textureSize);

        void CreateVertices();
        void CreateIndices();

        void UpdateTextureSize(Windows::Foundation::Size textureSize);

        void ResetBuffers();

        ID3D11Texture2D* GetReflectionTexture() const { return m_d3dReflectionTexture.Get(); }
        ID3D11ShaderResourceView* GetReflectionTextureResourceView() const { return m_d3dReflectionTextureShaderView.Get(); }
        ID3D11RenderTargetView1* GetReflectionRenderTarget() const { return m_d3dReflectionRenderTarget.Get(); }
        ID3D11DepthStencilView* GetReflectionDepthStencilView() const { return m_d3dReflectionDepthStencilView.Get(); }

        ID3D11Texture2D* GetRefractionTexture() const { return m_d3dRefractionTexture.Get(); }
        ID3D11ShaderResourceView* GetRefractionTextureResourceView() const { return m_d3dRefractionTextureShaderView.Get(); }
        ID3D11RenderTargetView1* GetRefractionRenderTarget() const { return m_d3dRefractionRenderTarget.Get(); }
        ID3D11DepthStencilView* GetRefractionDepthStencilView() const { return m_d3dRefractionDepthStencilView.Get(); }

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

        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_d3dReflectionTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_d3dReflectionTextureShaderView;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_d3dReflectionRenderTarget;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dReflectionDepthStencilView;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_d3dRefractionTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_d3dRefractionTextureShaderView;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView1> m_d3dRefractionRenderTarget;
        Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_d3dRefractionDepthStencilView;
    };
}