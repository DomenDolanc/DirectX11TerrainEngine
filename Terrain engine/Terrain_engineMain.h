#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\SceneRenderer.h"
#include "Content\TextRenderer.h"

namespace Terrain_engine
{
	class Terrain_engineMain : public DX::IDeviceNotify
	{
	public:
		Terrain_engineMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Terrain_engineMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
        void RenderFromCameraView();

		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

        void HandleKeyUpEvent(Windows::System::VirtualKey key);
        void HandleKeyDownEvent(Windows::System::VirtualKey key);
        void HandleMouseEvent(Windows::Foundation::Point point);

	private:
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		std::unique_ptr<SceneRenderer> m_sceneRenderer;
		std::unique_ptr<TextRenderer> m_textRenderer;

		DX::StepTimer m_timer;

        bool m_isCtrlKeyPressed;
	};
}