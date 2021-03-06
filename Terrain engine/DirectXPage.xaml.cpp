﻿//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace Terrain_engine;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;

DirectXPage::DirectXPage():
	m_windowVisible(true),
	m_coreInput(nullptr)
{
	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged += 
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

    window->KeyDown +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyDown);

    window->KeyUp +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &DirectXPage::OnKeyUp);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	// Register our SwapChainPanel to get independent input pointer events
	auto workItemHandler = ref new WorkItemHandler([this] (IAsyncAction ^)
	{
		// The CoreIndependentInputSource will raise pointer events for the specified device types on whichever thread it's created on.
		m_coreInput = swapChainPanel->CreateCoreIndependentInputSource(
			Windows::UI::Core::CoreInputDeviceTypes::Mouse |
			Windows::UI::Core::CoreInputDeviceTypes::Touch |
			Windows::UI::Core::CoreInputDeviceTypes::Pen
			);

		m_coreInput->PointerCursor = nullptr;

		// Register for pointer events, which will be raised on the background thread.
		m_coreInput->PointerPressed += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerPressed);
		m_coreInput->PointerMoved += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerMoved);
		m_coreInput->PointerReleased += ref new TypedEventHandler<Object^, PointerEventArgs^>(this, &DirectXPage::OnPointerReleased);

		// Begin processing input messages as they're delivered.
		m_coreInput->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
	});

	// Run task on a dedicated high priority background thread.
	m_inputLoopWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);

    InitSettingsWindow();

	m_main = std::unique_ptr<Terrain_engineMain>(new Terrain_engineMain(m_deviceResources));
	m_main->StartRenderLoop();
}

DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
}

void DirectXPage::OnPointerPressed(Object^ sender, PointerEventArgs^ e)
{
	// When the pointer is pressed begin tracking the pointer movement.
}

void DirectXPage::OnPointerMoved(Object^ sender, PointerEventArgs^ e)
{
    m_main->HandleMouseEvent(e->CurrentPoint->Position);
}

void DirectXPage::OnPointerReleased(Object^ sender, PointerEventArgs^ e)
{
	// Stop tracking pointer movement when the pointer is released.
}

void Terrain_engine::DirectXPage::InitSettingsWindow()
{
    if (m_settingsVisible)
    {
		SettingsPanel->Visibility = Windows::UI::Xaml::Visibility::Visible;
    }
    else
    {
		SettingsPanel->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
    }
}

void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetLogicalSize(e->NewSize);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    m_main->HandleKeyDownEvent(args->VirtualKey);
}

void DirectXPage::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    m_main->HandleKeyUpEvent(args->VirtualKey);
    if (args->VirtualKey == Windows::System::VirtualKey::Escape)
        m_settingsVisible = !m_settingsVisible;
    InitSettingsWindow();
}

void Terrain_engine::DirectXPage::LoadTextureButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	FileOpenPicker^ openPicker = ref new FileOpenPicker();
	openPicker->ViewMode = PickerViewMode::Thumbnail;
	openPicker->SuggestedStartLocation = PickerLocationId::PicturesLibrary;
	openPicker->FileTypeFilter->Append(".jpg");
	openPicker->FileTypeFilter->Append(".jpeg");
	openPicker->FileTypeFilter->Append(".png");
	openPicker->FileTypeFilter->Append(".tif");

	auto openFileTask = create_task(openPicker->PickSingleFileAsync()).then([this](StorageFile^ file)
	{
		if (file)
			return file;
	});
	openFileTask.then([this](task<StorageFile^> taskFile)
	{
		m_main->LoadBitmap(taskFile.get());
	});
}

void Terrain_engine::DirectXPage::LightPosSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
    if (!this->IsLoaded)
        return;

    LightPosXText->Text = "Position X: " + LightPosXSlider->Value.ToString();
    LightPosYText->Text = "Position Y: " + LightPosYSlider->Value.ToString();
    LightPosZText->Text = "Position Z: " + LightPosZSlider->Value.ToString();
    m_main->UpdateLightPosition({ (float)LightPosXSlider->Value, (float)LightPosYSlider->Value, (float)LightPosZSlider->Value });
}

void Terrain_engine::DirectXPage::TerrainOptionsSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
    if (!this->IsLoaded)
        return;

    TerrainColumnsText->Text = "Grid columns: " + TerrainColumnsSlider->Value.ToString();
	TerrainRowsText->Text = "Grid rows: " + TerrainRowsSlider->Value.ToString();
    TerrainAmplitudeText->Text = "Elevation: " + TerrainAmplitudeSlider->Value.ToString() + "m";
	TerrainTillingText->Text = "Tiling: " + TerrainTillingSlider->Value.ToString();

	TerrainParams params = { (float)TerrainColumnsSlider->Value , (float)TerrainRowsSlider->Value, 
							(float)TerrainAmplitudeSlider->Value, (float)TerrainTillingSlider->Value };
    m_main->UpdateTerrainSettings(params);
}


void Terrain_engine::DirectXPage::TessellationParams_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!this->IsLoaded)
		return;

	m_main->UseTessellation(UseTessellationCheckBox->IsChecked->Value);
	m_main->DrawLOD(DrawLODCheckBox->IsChecked->Value);
	m_main->UseFrustumCulling(UseFrustumCullingCheckBox->IsChecked->Value);
}

void Terrain_engine::DirectXPage::TextureParams_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!this->IsLoaded)
		return;

	m_main->UseTexture(UseTextureCheckBox->IsChecked->Value);
}

void Terrain_engine::DirectXPage::TerrainViewDistanceSlider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (!this->IsLoaded)
		return;

	TerrainViewDistanceText->Text = "View distance: " + TerrainViewDistanceSlider->Value.ToString() + "m";
	m_main->UpdateViewDistance(TerrainViewDistanceSlider->Value);
}

void Terrain_engine::DirectXPage::WaterParams_Changed(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (!this->IsLoaded)
		return;

	m_main->DrawWater(DrawWaterCheckBox->IsChecked->Value);
	m_main->ReflectWater(ReflectWaterCheckBox->IsChecked->Value);
	m_main->RefractWater(RefractWaterCheckBox->IsChecked->Value);
	m_main->UseWaterTessellation(UseWaterTesselationCheckBox->IsChecked->Value);
	m_main->UpdateWaveSpeed(WaveSpeedSlider->Value);
	m_main->UpdateWaveStrength(WaveStrengthSlider->Value);
}
