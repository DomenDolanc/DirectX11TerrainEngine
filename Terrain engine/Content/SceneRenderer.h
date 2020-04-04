#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "..\Common\StepTimer.h"
#include "..\Common\DirectXHelper.h"
#include "..\Common\PerlinNoise.h"
#include "..\Common\DirectXMesh.h"
#include <map>

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
        void MoveCameraForward();
        void MoveCameraBackward();
        void MoveCameraLeft();
        void MoveCameraRight();
        void UpdateMousePosition(DirectX::XMFLOAT2 mousePoint);
        void UpdateCamera();
        void UpdateCameraSpeed();
        void StopCameraMovement();

        void TogglePrimitiveRendering();
        void ToggleShadowsRendering();
        void SwitchTerrainPreset(int mode);

        void CreateVertices();

        DirectX::XMFLOAT3 GetColorFromHeight(double height);

        static const float INITIAL_TRAVEL_SPEED;

	private:
        void Render();
        void Translate(DirectX::XMFLOAT3 translation);

        void CreateIndices();

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader>	m_geometryShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer>		m_drawParamsConstantBuffer;

		ModelViewProjectionConstantBuffer	m_constantBufferData;
        DrawParamsConstantBuffer            m_drawParamsConstantBufferData;
		uint32	m_indexCount;

        VertexPositionColor* m_terrainVertices;
        uint32_t* m_terrainIndices;


        std::shared_ptr<PerlinNoise> m_PerlinNoise;
        static std::map<int, DirectX::XMFLOAT3> m_TerrainColormap;

        DirectX::XMVECTOR m_Eye = { 0.0f, 5000.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_At = { 0.0f, 0.0f, 0.0f, 0.0f };
        DirectX::XMVECTOR m_Up = { 0.0f, 1.0f, 0.0f, 0.0f };

        DirectX::XMFLOAT3 pos = {0.0f, 0.0f, 0.0f};
        double pitch;
        double yaw;
        float travelSpeed = INITIAL_TRAVEL_SPEED;
        const float acceleration = 0.1f;

        bool m_renderTriangles = true;
        bool m_renderShadows = true;

        DirectX::XMFLOAT2 m_MousePoint;

		bool	m_loadingComplete;
		float	m_degreesPerSecond;
		bool	m_tracking;
        const int m_Columns = 200;
        const int m_Rows = 200;
        const float m_sceneScaling = 10000;
	};
}

