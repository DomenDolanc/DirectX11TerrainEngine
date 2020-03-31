#include "pch.h"
#include "Terrain_engineMain.h"
#include "Common\DirectXHelper.h"

using namespace Terrain_engine;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace Microsoft::WRL;

Terrain_engineMain::Terrain_engineMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	m_deviceResources->RegisterDeviceNotify(this);
	m_sceneRenderer = std::unique_ptr<SceneRenderer>(new SceneRenderer(m_deviceResources));
	m_textRenderer = std::unique_ptr<TextRenderer>(new TextRenderer(m_deviceResources));
}

Terrain_engineMain::~Terrain_engineMain()
{
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

void Terrain_engineMain::CreateWindowSizeDependentResources() 
{
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void Terrain_engineMain::Update() 
{
	m_timer.Tick([&]()
	{
		m_sceneRenderer->Update(m_timer);
		m_textRenderer->Update(m_timer);
	});
}

bool Terrain_engineMain::Render() 
{
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();
    auto context2d = m_deviceResources->GetD2DDeviceContext();

	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);


    ID3D11SamplerState* const sampler[1] = { m_deviceResources->GetSampler() };
    context->PSSetSamplers(0, 1, sampler);

    RenderFromLightsView();


    RenderFromCameraView();
	
	m_textRenderer->Render();

	return true;
}

void Terrain_engine::Terrain_engineMain::RenderFromCameraView()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };

    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    ID3D11ShaderResourceView* const resourceView[1] = { m_deviceResources->GetShadowMapShaderResourceView() };
    context->PSSetShaderResources(0, 1, resourceView);

    m_sceneRenderer->RenderFromCameraView();
}

void Terrain_engine::Terrain_engineMain::RenderFromLightsView()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    ID3D11ShaderResourceView* const resourceViewNull[1] = { nullptr };
    ID3D11RenderTargetView* const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->PSSetShaderResources(0, 1, resourceViewNull);

    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetShadowMapDepthView());
    context->ClearDepthStencilView(m_deviceResources->GetShadowMapDepthView(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_sceneRenderer->RenderFromLightsView();
    context->PSSetShaderResources(0, 1, resourceViewNull);
}

void Terrain_engineMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_textRenderer->ReleaseDeviceDependentResources();
}

void Terrain_engineMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_textRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

void Terrain_engine::Terrain_engineMain::HandleKeyUpEvent(Windows::System::VirtualKey key)
{
    switch (key)
    {
        case Windows::System::VirtualKey::Tab:
            m_textRenderer->HideHelpDisplay();
            break;
        case Windows::System::VirtualKey::Control:
            m_isCtrlKeyPressed = false;
            break;
        default:
            m_sceneRenderer->StopCameraMovement();
    }
}

void Terrain_engine::Terrain_engineMain::HandleKeyDownEvent(Windows::System::VirtualKey key)
{
    if (m_isCtrlKeyPressed && key == Windows::System::VirtualKey::W)
    {
        m_sceneRenderer->TogglePrimitiveRendering();
        return;
    }

    if (m_isCtrlKeyPressed && key == Windows::System::VirtualKey::S)
    {
        m_sceneRenderer->ToggleShadowsRendering();
        return;
    }

    if (key >= Windows::System::VirtualKey::Number0 && key <= Windows::System::VirtualKey::Number9)
    {
        m_sceneRenderer->SwitchTerrainPreset(static_cast<int>(key - Windows::System::VirtualKey::Number0));
    }

    switch (key)
    {
        case Windows::System::VirtualKey::W:
            m_sceneRenderer->MoveCameraForward();
            break;
        case Windows::System::VirtualKey::S:
            m_sceneRenderer->MoveCameraBackward();
            break;
        case Windows::System::VirtualKey::A:
            m_sceneRenderer->MoveCameraLeft();
            break;
        case Windows::System::VirtualKey::D:
            m_sceneRenderer->MoveCameraRight();
            break;
        case Windows::System::VirtualKey::Up:
            m_sceneRenderer->MoveLightUp();
            break;
        case Windows::System::VirtualKey::Down:
            m_sceneRenderer->MoveLightDown();
            break;
        case Windows::System::VirtualKey::Left:
            m_sceneRenderer->MoveLightForward();
            break;
        case Windows::System::VirtualKey::Right:
            m_sceneRenderer->MoveLightBackward();
            break;
        case Windows::System::VirtualKey::Tab:
            m_textRenderer->ShowHelpDisplay();
            break;
        case Windows::System::VirtualKey::Control:
            m_isCtrlKeyPressed = true;
            break;
    }
}

void Terrain_engine::Terrain_engineMain::HandleMouseEvent(Windows::Foundation::Point point)
{
    DirectX::XMFLOAT2 point2D{point.X, point.Y};
    m_sceneRenderer->UpdateMousePosition(point2D);
}
