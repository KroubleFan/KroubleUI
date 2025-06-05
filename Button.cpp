#include "KroubleUI.h"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

namespace KroubleUI {

    Button::Button(Window* parent, const D2D1_RECT_F& rect, const std::wstring& text)
        : Control(parent, rect),
        m_text(text),
        m_textBrush(nullptr),
        m_backgroundBrush(nullptr),
        m_borderBrush(nullptr),
        m_textFormat(nullptr),
        m_isHovered(false),
        m_isPressed(false) {
    }

    Button::~Button() {
        SafeReleaseResources();
    }

    void Button::Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory) {
        SafeReleaseResources();

        // 创建默认画笔
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::LightGray), &m_backgroundBrush);
        renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), &m_borderBrush);

        // 创建文本格式
        dwriteFactory->CreateTextFormat(
            L"Microsoft YaHei",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            14.0f,
            L"zh-cn",
            &m_textFormat
        );

        if (m_textFormat) {
            m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        }
    }

    void Button::SafeReleaseResources() {
        if (m_textBrush) m_textBrush->Release();
        if (m_backgroundBrush) m_backgroundBrush->Release();
        if (m_borderBrush) m_borderBrush->Release();
        if (m_textFormat) m_textFormat->Release();

        m_textBrush = nullptr;
        m_backgroundBrush = nullptr;
        m_borderBrush = nullptr;
        m_textFormat = nullptr;
    }

    void Button::Draw(ID2D1RenderTarget* renderTarget) {
        if (!m_visible) return;

        // 初始化资源（如果尚未初始化）
        if (!m_textBrush && renderTarget && m_parent) {
            IDWriteFactory* dwriteFactory = m_parent->GetDWriteFactory();
            if (dwriteFactory) {
                Initialize(renderTarget, dwriteFactory);
            }
        }

        // 确定背景和边框颜色（根据状态）
        D2D1_COLOR_F bgColor = m_backgroundBrush ? m_backgroundBrush->GetColor() : D2D1::ColorF(D2D1::ColorF::LightGray);
        D2D1_COLOR_F borderColor = m_borderBrush ? m_borderBrush->GetColor() : D2D1::ColorF(D2D1::ColorF::DarkGray);

        if (m_isPressed) {
            bgColor.r *= 0.8f;
            bgColor.g *= 0.8f;
            bgColor.b *= 0.8f;
        }
        else if (m_isHovered) {
            bgColor.r *= 1.1f;
            bgColor.g *= 1.1f;
            bgColor.b *= 1.1f;
        }

        // 绘制背景
        ID2D1SolidColorBrush* tempBgBrush = nullptr;
        renderTarget->CreateSolidColorBrush(bgColor, &tempBgBrush);
        renderTarget->FillRectangle(m_rect, tempBgBrush);
        tempBgBrush->Release();

        // 绘制边框
        ID2D1SolidColorBrush* tempBorderBrush = nullptr;
        renderTarget->CreateSolidColorBrush(borderColor, &tempBorderBrush);
        renderTarget->DrawRectangle(m_rect, tempBorderBrush, 1.0f);
        tempBorderBrush->Release();

        // 绘制文本
        if (m_textBrush && m_textFormat && !m_text.empty()) {
            renderTarget->DrawTextW(
                m_text.c_str(),
                static_cast<UINT32>(m_text.length()),
                m_textFormat,
                m_rect,
                m_textBrush
            );
        }
    }

    void Button::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        bool isInside = (pt.x >= m_rect.left && pt.x <= m_rect.right &&
            pt.y >= m_rect.top && pt.y <= m_rect.bottom);

        switch (message) {
        case WM_MOUSEMOVE:
            m_isHovered = isInside;
            break;

        case WM_LBUTTONDOWN:
            if (isInside) {
                m_isPressed = true;
                SetCapture(m_parent->GetHwnd());
            }
            break;

        case WM_LBUTTONUP:
            if (m_isPressed) {
                m_isPressed = false;
                ReleaseCapture();

                if (isInside && m_onClickHandler) {
                    m_onClickHandler();
                }
            }
            break;

        case WM_MOUSELEAVE:
            m_isHovered = false;
            m_isPressed = false;
            break;
        }
    }

    void Button::SetText(const std::wstring& text) {
        m_text = text;
    }

    const std::wstring& Button::GetText() const {
        return m_text;
    }

    void Button::SetOnClickHandler(std::function<void()> handler) {
        m_onClickHandler = handler;
    }

    void Button::SetTextColor(const D2D1_COLOR_F& color) {
        if (m_textBrush) {
            m_textBrush->SetColor(color);
        }
    }

    void Button::SetBackgroundColor(const D2D1_COLOR_F& color) {
        if (m_backgroundBrush) {
            m_backgroundBrush->SetColor(color);
        }
    }

    void Button::SetBorderColor(const D2D1_COLOR_F& color) {
        if (m_borderBrush) {
            m_borderBrush->SetColor(color);
        }
    }

} // namespace KroubleUI