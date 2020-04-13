#include "pch.h"
#include "Terrain_engineMain.h"
#include "Common\DirectXHelper.h"
#include "Common/DirectXHelpers/BasicLoader.h"

using namespace Terrain_engine;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;
using namespace Windows::Storage;

// Loads and initializes application assets when the application is loaded.
Terrain_engineMain::Terrain_engineMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources), m_pointerLocationX(0.0f)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	// TODO: Replace this with your app's content initialization.
	m_sceneRenderer = std::unique_ptr<SceneRenderer>(new SceneRenderer(m_deviceResources));

	m_textRenderer = std::unique_ptr<TextRenderer>(new TextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

Terrain_engineMain::~Terrain_engineMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void Terrain_engineMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_sceneRenderer->CreateWindowSizeDependentResources();
}

void Terrain_engineMain::StartRenderLoop()
{
	// If the animation render loop is already running then do not start another thread.
	if (m_renderLoopWorker != nullptr && m_renderLoopWorker->Status == AsyncStatus::Started)
	{
		return;
	}

	// Create a task that will be run on a background thread.
	auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction ^ action)
	{
		// Calculate the updated frame and render once per vertical blanking interval.
		while (action->Status == AsyncStatus::Started)
		{
			critical_section::scoped_lock lock(m_criticalSection);
			Update();
			if (Render())
			{
				m_deviceResources->Present();
			}
		}
	});

	// Run task on a dedicated high priority background thread.
	m_renderLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

void Terrain_engineMain::StopRenderLoop()
{
	m_renderLoopWorker->Cancel();
}

void Terrain_engine::Terrain_engineMain::HandleKeyUpEvent(Windows::System::VirtualKey key)
{
    switch (key)
    {
    case Windows::System::VirtualKey::Control:
        m_isCtrlKeyPressed = false;
        break;
    default:
        m_sceneRenderer->getCamera()->Stop();
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

    switch (key)
    {
    case Windows::System::VirtualKey::W:
    case Windows::System::VirtualKey::Up:
        m_sceneRenderer->getCamera()->MoveForward();
        break;
    case Windows::System::VirtualKey::S:
    case Windows::System::VirtualKey::Down:
        m_sceneRenderer->getCamera()->MoveBackward();
        break;
    case Windows::System::VirtualKey::A:
    case Windows::System::VirtualKey::Left:
        m_sceneRenderer->getCamera()->MoveLeft();
        break;
    case Windows::System::VirtualKey::D:
    case Windows::System::VirtualKey::Right:
        m_sceneRenderer->getCamera()->MoveRight();
        break;
    case Windows::System::VirtualKey::Escape:
        m_isEscKeyPressed = !m_isEscKeyPressed;
        break;
    case Windows::System::VirtualKey::Control:
        m_isCtrlKeyPressed = true;
        break;
    }
}

void Terrain_engine::Terrain_engineMain::HandleMouseEvent(Windows::Foundation::Point point)
{
    DirectX::XMFLOAT2 point2D{ point.X, point.Y };
    if (m_isEscKeyPressed)
        return;

    m_sceneRenderer->UpdateMousePosition(point2D);
}

void Terrain_engine::Terrain_engineMain::UpdateLightPosition(DirectX::XMFLOAT3 lightPos)
{
    m_sceneRenderer->UpdateLightPosition(lightPos);
}

void Terrain_engine::Terrain_engineMain::UpdateTerrainSettings(TerrainParams params)
{
    m_sceneRenderer->UpdateTerrainSettings(params);
}

void Terrain_engine::Terrain_engineMain::UseTessellation(bool useTessellation)
{
    m_sceneRenderer->UseTessellation(useTessellation);
}

void Terrain_engine::Terrain_engineMain::DrawLOD(bool drawLOD)
{
    m_sceneRenderer->DrawLOD(drawLOD);
}

void Terrain_engine::Terrain_engineMain::LoadBitmap(StorageFile^ file)
{
    auto tempFolder = Windows::Storage::ApplicationData::Current->TemporaryFolder;
    create_task(file->CopyAsync(tempFolder, file->Name, NameCollisionOption::GenerateUniqueName)).then([this](StorageFile^ tempFile)
    {
        if (tempFile)
        {
            BasicLoader^ basicLoader = ref new BasicLoader(m_deviceResources->GetD3DDevice(), m_deviceResources->GetWicImagingFactory());

            ID3D11Texture2D* terrainTexture;
            ID3D11ShaderResourceView* terrainShaderResourceView;
            Platform::String^ fileName = tempFile->Path;

            basicLoader->LoadTexture(fileName, &terrainTexture, &terrainShaderResourceView);
            m_deviceResources->SetTerrainHeightMap(terrainTexture);
            m_deviceResources->SetTerrainHeightShaderResourceView(terrainShaderResourceView);
            D3D11_TEXTURE2D_DESC desc;
            terrainTexture->GetDesc(&desc);
            m_sceneRenderer->SetTextureSize(desc.Width, desc.Height);
            DeleteFile(tempFile->Path->Data());
        }
    });
}

// Updates the application state once per frame.
void Terrain_engineMain::Update() 
{
	ProcessInput();

	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_sceneRenderer->Update(m_timer);
		m_textRenderer->Update(m_timer);
	});
}

// Process all input from the user before updating game state
void Terrain_engineMain::ProcessInput()
{
	// TODO: Add per frame input handling here.
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool Terrain_engineMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0 || !m_sceneRenderer->IsReadyToRender())
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    ID3D11SamplerState* const sampler[1] = { m_deviceResources->GetSampler() };
    context->PSSetSamplers(0, 1, sampler);

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

// Notifies renderers that device resources need to be released.
void Terrain_engineMain::OnDeviceLost()
{
	m_sceneRenderer->ReleaseDeviceDependentResources();
	m_textRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void Terrain_engineMain::OnDeviceRestored()
{
	m_sceneRenderer->CreateDeviceDependentResources();
	m_textRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
