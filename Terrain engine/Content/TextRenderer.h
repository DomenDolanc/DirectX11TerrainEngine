#pragma once

#include <string>
#include "..\Common\DeviceResources.h"
#include "..\Common\StepTimer.h"

namespace Terrain_engine
{
    class TextRenderer
    {
    public:
        TextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        void CreateDeviceDependentResources();
        void ReleaseDeviceDependentResources();
        void Update(DX::StepTimer const& timer);
        void ShowHelpDisplay();
        void HideHelpDisplay();
        void Render();
        void DrawHelpDisplay();

        void BeginDraw();
        void EndDraw();
        void DrawText(std::wstring text, int x, int y);

    private:
        std::shared_ptr<DX::DeviceResources> m_deviceResources;


        bool m_DrawHelpDisplay = false;
        std::wstring                                    m_FPSText;
        DWRITE_TEXT_METRICS	                            m_textMetrics;
        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
        Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
        Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
        Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;
    };
}