#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\Graphics\Terrain.h"
#include "..\Common\Graphics\Sphere.h"
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

        void UpdateLightPosition(DirectX::XMFLOAT3 lightPos);
        void UpdateTerrainSettings(DirectX::XMFLOAT3 terrainParams);
        void UseTessellation(bool useTessellation);
        void UpdateTesselationParams(DirectX::XMFLOAT4 tessellationParams);
        void DrawLOD(bool drawLOD);

        std::shared_ptr<Camera> getCamera();

    private:
        void Render();

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11HullShader>	    m_hullShader;
        Microsoft::WRL::ComPtr<ID3D11DomainShader>	    m_domainShader;

        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_constantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_drawParamsConstantBuffer;

        ModelViewProjectionConstantBuffer	m_constantBufferData;
        DrawParamsConstantBuffer            m_drawParamsConstantBufferData;

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Terrain> m_Terrain;
        std::shared_ptr<Sphere> m_Light;

        DirectX::XMFLOAT3 m_lightPos;

        bool m_renderTriangles = true;
        bool m_renderShadows = true;
        bool m_usesTessellation = false;
        bool m_drawLOD = false;

        DirectX::XMFLOAT2 m_MousePoint;

        bool	m_loadingComplete;
        float	m_degreesPerSecond;
        bool	m_tracking;
        const float m_sceneScaling = 10000;
    };
}

