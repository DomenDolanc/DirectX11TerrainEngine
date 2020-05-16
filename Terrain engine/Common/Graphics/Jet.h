#pragma once

#include "..\Common\DeviceResources.h"
#include "Content/ShaderStructures.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\DirectXMesh.h"

namespace Terrain_engine
{
    class Loader;

    class Jet
    {
    public:
        Jet(std::shared_ptr<DX::DeviceResources> deviceResources);
        ~Jet();

        void LoadObject();

        void ResetBuffers();

        DirectX::XMMATRIX GetTransformation() const;

        ID3D11Texture2D* GetTexture() const { return m_d3dTexture.Get(); }
        ID3D11ShaderResourceView* GetTextureShaderResourceView() const { return m_d3dTextureShaderView.Get(); }

        void Draw();
    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        std::shared_ptr<Terrain_engine::Loader> m_objLoader;

        DirectX::XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };

        float m_scaling = 1.0f;

        bool m_loadingComplete = true;

        Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_d3dTexture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>m_d3dTextureShaderView;
    };
}