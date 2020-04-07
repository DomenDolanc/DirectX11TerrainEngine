#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\Graphics\Terrain.h"
#include "..\Common\Graphics\Camera.h"
#include "..\Common\DirectXMesh.h"

namespace Terrain_engine
{
    class SceneRenderer
    {
    public:
        SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        ~SceneRenderer();
        void CreateDeviceDependentResources();
        void CreateWindowSizeDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void RenderFromCameraView();

        void UpdateMousePosition(DirectX::XMFLOAT2 mousePoint);

        void TogglePrimitiveRendering();
        void ToggleShadowsRendering();
        void SwitchTerrainPreset(int mode);

        std::shared_ptr<Camera> getCamera();

    private:
        void Render();

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_constantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_drawParamsConstantBuffer;

        ModelViewProjectionConstantBuffer	m_constantBufferData;
        DrawParamsConstantBuffer            m_drawParamsConstantBufferData;

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Terrain> m_Terrain;

        bool m_renderTriangles = true;
        bool m_renderShadows = true;

        DirectX::XMFLOAT2 m_MousePoint;

        bool	m_loadingComplete;
        float	m_degreesPerSecond;
        bool	m_tracking;
        const float m_sceneScaling = 10000;
    };
}

