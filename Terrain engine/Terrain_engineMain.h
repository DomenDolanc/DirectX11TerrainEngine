#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\SceneRenderer.h"
#include "Content\TextRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace Terrain_engine
{
	class Terrain_engineMain : public DX::IDeviceNotify
	{
	public:
		Terrain_engineMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Terrain_engineMain();
		void CreateWindowSizeDependentResources();
		void StartRenderLoop();
		void StopRenderLoop();
		Concurrency::critical_section& GetCriticalSection() { return m_criticalSection; }


        void HandleKeyUpEvent(Windows::System::VirtualKey key);
        void HandleKeyDownEvent(Windows::System::VirtualKey key);
        void HandleMouseEvent(Windows::Foundation::Point point);

        void UpdateLightPosition(DirectX::XMFLOAT3 lightPos);
        void UpdateTerrainSettings(TerrainParams params);
		void UseTessellation(bool useTessellation);
		void UseTexture(bool useTexture);
		void DrawLOD(bool drawLOD);
		void UseFrustumCulling(bool useFrustumCulling);
		void LoadBitmap(Windows::Storage::StorageFile^ file);
		void UpdateViewDistance(double viewDistance);

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		void ProcessInput();
		void Update();
		bool Render();

		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<SceneRenderer> m_sceneRenderer;
		std::unique_ptr<TextRenderer> m_textRenderer;

		Windows::Foundation::IAsyncAction^ m_renderLoopWorker;
		Concurrency::critical_section m_criticalSection;

		// Rendering loop timer.
		DX::StepTimer m_timer;

		// Track current input pointer position.
		float m_pointerLocationX;

        bool m_isCtrlKeyPressed;
        bool m_isEscKeyPressed = false;
	};
}