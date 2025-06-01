#pragma once
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <stdexcept>
#include <windowsx.h>
#include <imm.h>
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

namespace KroubleUI {

	template<class T> void SafeRelease(T** ppT) {
		if (*ppT) {
			(*ppT)->Release();
			*ppT = nullptr;
		}
	}

	// 前向声明
	class Window;
	class TextBox;

	// 基础控件类
	class Control {
	protected:
		Window* m_parent;
		D2D1_RECT_F m_rect;
		bool m_visible;

	public:
        // 添加这个方法
        virtual bool HitTest(float x, float y) const {
            return x >= m_rect.left && x <= m_rect.right &&
                y >= m_rect.top && y <= m_rect.bottom;
        }

        virtual void Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory) = 0;
		Control(Window* parent, const D2D1_RECT_F& rect)
			: m_parent(parent), m_rect(rect), m_visible(true) {
		}
		virtual ~Control() = default;

		virtual void Draw(ID2D1RenderTarget* renderTarget) = 0;
		virtual void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {}
		virtual void OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam) {}

		void SetRect(const D2D1_RECT_F& rect) { m_rect = rect; }
		const D2D1_RECT_F& GetRect() const { return m_rect; }
		void SetVisible(bool visible) { m_visible = visible; }
		bool IsVisible() const { return m_visible; }
	};

	// 文本输入框类
	class TextBox : public Control {
	private:
		std::wstring m_text;
		bool m_hasFocus;
		bool m_isComposing;
		std::wstring m_compositionString;
		ID2D1SolidColorBrush* m_borderBrush;
		ID2D1SolidColorBrush* m_backgroundBrush;
		ID2D1SolidColorBrush* m_textBrush;
		ID2D1SolidColorBrush* m_compositionBrush;
		IDWriteTextFormat* m_textFormat;
	public:
        TextBox(Window* parent, const D2D1_RECT_F& rect, const std::wstring& initialText = L"");
			

		~TextBox() {
			SafeRelease(&m_borderBrush);
			SafeRelease(&m_backgroundBrush);
			SafeRelease(&m_textBrush);
			SafeRelease(&m_compositionBrush);
			SafeRelease(&m_textFormat);
		}


        virtual void Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory);

		void Draw(ID2D1RenderTarget* renderTarget) override;

		void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) override;

		void OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam) override;

		const std::wstring& GetText() const { return m_text; }
		void SetText(const std::wstring& text);

	private:
		bool PtInRectF(const D2D1_RECT_F& rect, POINT pt) {
			return pt.x >= rect.left && pt.x <= rect.right &&
				pt.y >= rect.top && pt.y <= rect.bottom;
		}	
	};

    // 在 KroubleUI 命名空间中添加
    class TextBlock : public Control {
    private:
        std::wstring m_text;
        ID2D1SolidColorBrush* m_textBrush;
        ID2D1SolidColorBrush* m_backgroundBrush;  // 新增背景画笔
        IDWriteTextFormat* m_textFormat;
        bool m_wordWrap;
        DWRITE_TEXT_ALIGNMENT m_textAlignment;
        DWRITE_PARAGRAPH_ALIGNMENT m_paragraphAlignment;
        IDWriteFactory* m_dwriteFactory;
    public:
        TextBlock(Window* parent, const D2D1_RECT_F& rect, const std::wstring& text = L"");

        ~TextBlock() {
            if (m_textBrush) m_textBrush->Release();
            if (m_backgroundBrush) m_backgroundBrush->Release();  // 释放背景画笔
            if (m_textFormat) m_textFormat->Release();

        }

        virtual void Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory);

        void Draw(ID2D1RenderTarget* renderTarget) override;

        // 设置文本内容
        void SetText(const std::wstring& text) {
            m_text = text;
        }

        // 获取文本内容
        const std::wstring& GetText() const {
            return m_text;
        }

        // 设置是否自动换行
        void SetWordWrap(bool wrap) {
            if (m_wordWrap != wrap) {
                m_wordWrap = wrap;
                UpdateTextFormat();
            }
        }

        // 设置文本水平对齐方式
        void SetTextAlignment(DWRITE_TEXT_ALIGNMENT alignment) {
            if (m_textAlignment != alignment) {
                m_textAlignment = alignment;
                UpdateTextFormat();
            }
        }

        // 设置文本垂直对齐方式
        void SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT alignment) {
            if (m_paragraphAlignment != alignment) {
                m_paragraphAlignment = alignment;
                UpdateTextFormat();
            }
        }

        // 设置文本颜色
        void SetTextColor(const D2D1_COLOR_F& color) {
            if (m_textBrush) {
                m_textBrush->SetColor(color);
            }
        }
        void SetBackgroundColor(const D2D1_COLOR_F& color);
        // 设置字体大小
        void SetFontSize(float size) {
            if (m_textFormat) {
                if (m_textFormat->GetFontSize() != size)
                {
                    // 创建文本格式
                    m_dwriteFactory->CreateTextFormat(
                        L"Microsoft YaHei", // 支持中文的字体
                        nullptr,
                        DWRITE_FONT_WEIGHT_NORMAL,
                        DWRITE_FONT_STYLE_NORMAL,
                        DWRITE_FONT_STRETCH_NORMAL,
                        size,
                        L"zh-cn", // 中文locale
                        &m_textFormat
                    );
                }
            }
        }

    private:
        void UpdateTextFormat() {
            if (m_textFormat) {
                m_textFormat->SetWordWrapping(m_wordWrap ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);
                m_textFormat->SetTextAlignment(m_textAlignment);
                m_textFormat->SetParagraphAlignment(m_paragraphAlignment);
            }
        }

        template<class T> void SafeRelease(T** ppT) {
            if (*ppT) {
                (*ppT)->Release();
                *ppT = nullptr;
            }
        }
    };


    class Button : public Control {
    private:
        std::wstring m_text;
        ID2D1SolidColorBrush* m_textBrush;
        ID2D1SolidColorBrush* m_backgroundBrush;
        ID2D1SolidColorBrush* m_borderBrush;
        IDWriteTextFormat* m_textFormat;

        bool m_isHovered;
        bool m_isPressed;

        std::function<void()> m_onClickHandler;

        virtual void Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory);
        void SafeReleaseResources();

    public:
        Button(Window* parent, const D2D1_RECT_F& rect, const std::wstring& text = L"Button");
        virtual ~Button();

        // Control 接口实现
        void Draw(ID2D1RenderTarget* renderTarget) override;
        void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) override;

        // Button 特有方法
        void SetText(const std::wstring& text);
        const std::wstring& GetText() const;

        void SetOnClickHandler(std::function<void()> handler);

        // 样式设置
        void SetTextColor(const D2D1_COLOR_F& color);
        void SetBackgroundColor(const D2D1_COLOR_F& color);
        void SetBorderColor(const D2D1_COLOR_F& color);
    };

	// 窗口类
	class Window {
	private:
		HWND m_hwnd;
		ID2D1Factory* m_d2dFactory;
		IDWriteFactory* m_dwriteFactory;
		ID2D1HwndRenderTarget* m_renderTarget;
		std::vector<std::unique_ptr<Control>> m_controls;

	public:
		Window(HINSTANCE hInstance, const std::wstring& title, int width, int height);

		~Window() {
			SafeRelease(&m_renderTarget);
			SafeRelease(&m_dwriteFactory);
			SafeRelease(&m_d2dFactory);
			DestroyWindow(m_hwnd);
		}

		ID2D1HwndRenderTarget* GetRenderTarget() const { return m_renderTarget; }
		IDWriteFactory* GetDWriteFactory() const { return m_dwriteFactory; }

		void AddControl(Control* control);

		void RunMessageLoop();

		void Render();

		void OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam);

		void OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam);

		HWND GetHwnd() const { return m_hwnd; }

	private:
		void InitializeDirect2D();

		void DiscardGraphicsResources();
		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
} // namespace KroubleUI