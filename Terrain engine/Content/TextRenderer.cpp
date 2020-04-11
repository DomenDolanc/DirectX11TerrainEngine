#include "pch.h"
#include "TextRenderer.h"

#include "Common/DirectXHelper.h"

using namespace Terrain_engine;
using namespace Microsoft::WRL;

TextRenderer::TextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_FPSText(L""),
    m_deviceResources(deviceResources)
{
    ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

    ComPtr<IDWriteTextFormat> textFormat;
    DX::ThrowIfFailed(
        m_deviceResources->GetDWriteFactory()->CreateTextFormat(
            L"Segoe UI",
            nullptr,
            DWRITE_FONT_WEIGHT_LIGHT,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            18.0f,
            L"en-US",
            &textFormat
        )
    );

    DX::ThrowIfFailed(textFormat.As(&m_textFormat));

    DX::ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR));

    DX::ThrowIfFailed(m_deviceResources->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock));

    CreateDeviceDependentResources();
}

void TextRenderer::Update(DX::StepTimer const& timer)
{
    uint32 fps = timer.GetFramesPerSecond();
    m_FPSText = (fps > 0) ? std::to_wstring(fps) + L" FPS" : L" - FPS";
}

void TextRenderer::BeginDraw()
{
    ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();

    context->SaveDrawingState(m_stateBlock.Get());
    context->BeginDraw();
}

void TextRenderer::EndDraw()
{
    ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
    HRESULT hr = context->EndDraw();
    if (hr != D2DERR_RECREATE_TARGET)
    {
        DX::ThrowIfFailed(hr);
    }

    context->RestoreDrawingState(m_stateBlock.Get());
}

void TextRenderer::DrawText(std::wstring text, int x, int y)
{
    Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

    ID2D1DeviceContext* context = m_deviceResources->GetD2DDeviceContext();
    ComPtr<IDWriteTextLayout> textLayout;
    DX::ThrowIfFailed(
        m_deviceResources->GetDWriteFactory()->CreateTextLayout(
            text.c_str(),
            (uint32)text.length(),
            m_textFormat.Get(),
            logicalSize.Width,
            logicalSize.Height,
            &textLayout
        )
    );

    DX::ThrowIfFailed(textLayout.As(&m_textLayout));
    DX::ThrowIfFailed(m_textLayout->GetMetrics(&m_textMetrics));

    D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation((float)x, (float)y);

    context->SetTransform(screenTranslation * m_deviceResources->GetOrientationTransform2D());
    DX::ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));

    context->DrawTextLayout(D2D1::Point2F(0.f, 0.f), m_textLayout.Get(), m_whiteBrush.Get());

}

void TextRenderer::Render()
{
    Windows::Foundation::Size logicalSize = m_deviceResources->GetLogicalSize();

    BeginDraw();
    DrawText(m_FPSText, logicalSize.Width - 100, 0);

    EndDraw();
}

void TextRenderer::CreateDeviceDependentResources()
{
    DX::ThrowIfFailed(m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush));
}
void TextRenderer::ReleaseDeviceDependentResources()
{
    m_whiteBrush.Reset();
}