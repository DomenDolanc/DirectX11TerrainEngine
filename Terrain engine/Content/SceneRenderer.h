﻿#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\Graphics\Terrain.h"
#include "..\Common\Graphics\Sphere.h"
#include "..\Common\Graphics\Camera.h"
#include "..\Common\Graphics\Water.h"
#include "..\Common\Graphics\Skybox.h"
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
        void Render();

        void UpdateMousePosition(DirectX::XMFLOAT2 mousePoint);
        void SetMousePosition(DirectX::XMFLOAT2 mousePoint);

        void TogglePrimitiveRendering();
        void ToggleShadowsRendering();

        void UpdateLightPosition(DirectX::XMFLOAT3 lightPos);
        void UpdateTerrainSettings(TerrainParams params);
        void UseTessellation(bool useTessellation);
        void UseTexture(bool useTexture);
        void DrawLOD(bool drawLOD);
        void UseFrustumCulling(bool useFrustumCulling);
        void SetTextureSize(int width, int height);
        void UpdateViewDistance(double viewDistance);
        void GetViewFrustum(DirectX::XMFLOAT4 planes[6]);

        void DrawWater(bool drawWater);
        void ReflectWater(bool reflectWater);
        void RefractWater(bool refractWater);
        void UseWaterTessellation(bool useWaterTessellation);

        bool IsReadyToRender();

        std::shared_ptr<Camera> getCamera();

    private:
        void RenderToBackBuffer();
        void RenderToWaterReflection();
        void RenderToWaterRefraction();

        void RenderScene();
        void RenderWater();
        void RenderTerrain();
        void RenderLightSource();
        void RenderSkybox();

        void SetProjection(double viewDistance);

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        Microsoft::WRL::ComPtr<ID3D11InputLayout>	    m_inputLayout;
        Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11HullShader>	    m_hullShader;
        Microsoft::WRL::ComPtr<ID3D11DomainShader>	    m_domainShader;

        Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_waterVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_waterPixelShader;

        Microsoft::WRL::ComPtr<ID3D11VertexShader>	    m_skyboxVertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>	    m_skyboxPixelShader;

        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_constantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_drawParamsConstantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		    m_waterParamsConstantBuffer;

        ModelViewProjectionConstantBuffer	m_constantBufferData;
        DrawParamsConstantBuffer            m_drawParamsConstantBufferData;
        WaterParamsConstantBuffer            m_waterParamsConstantBufferData;

        std::shared_ptr<Camera> m_Camera;
        std::shared_ptr<Terrain> m_Terrain;
        std::shared_ptr<Sphere> m_Light;
        std::shared_ptr<Water> m_Water;
        std::shared_ptr<Skybox> m_Skybox;

        DirectX::XMFLOAT3 m_lightPos;

        bool m_renderTriangles = true;
        bool m_renderShadows = true;
        bool m_usesTessellation = false;
        bool m_drawLOD = false;
        bool m_useFrustumCulling = false;

        bool m_drawWater = false;
        bool m_refractWater = false;
        bool m_reflectWater = false;
        bool m_useWaterTessellation = false;

        DirectX::XMFLOAT2 m_MousePoint;

        bool	m_loadingComplete;
        float	m_degreesPerSecond;
        bool	m_tracking;
        const float m_sceneScaling = 50000;
        float m_viewDistance = 0.0f;
        float m_waveMoveFactor = 0.0f;
    };
}

