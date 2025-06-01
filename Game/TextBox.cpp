#include "KroubleUI.h"

namespace KroubleUI {
	TextBox::TextBox(Window* parent, const D2D1_RECT_F& rect, const std::wstring& initialText)
		: Control(parent, rect), m_text(initialText), m_hasFocus(false),
		m_isComposing(false), m_borderBrush(nullptr), m_backgroundBrush(nullptr),
		m_textBrush(nullptr), m_compositionBrush(nullptr), m_textFormat(nullptr){
		Initialize(parent->GetRenderTarget(), parent->GetDWriteFactory());
	}

	void TextBox::Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory) {
		// Create brushes
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_borderBrush);
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_backgroundBrush);
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush);
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray), &m_compositionBrush);

		// Create text format
		dwriteFactory->CreateTextFormat(
			L"Microsoft YaHei", // Use a font that supports Chinese characters
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			14.0f,
			L"zh-cn",          // Set locale to Chinese
			&m_textFormat
		);

		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}

	void TextBox::Draw(ID2D1RenderTarget* renderTarget) {
		if (!m_visible) return;
		// 绘制背景和边框...
		renderTarget->FillRectangle(m_rect, m_backgroundBrush);
		renderTarget->DrawRectangle(m_rect, m_borderBrush, m_hasFocus ? 2.0f : 1.0f);
		D2D1_RECT_F textRect = m_rect;
		// 绘制文本
		if (!m_text.empty()) {
			textRect.left += 5.0f;
			textRect.right -= 5.0f;
			renderTarget->DrawTextW(
				m_text.c_str(),
				static_cast<UINT32>(m_text.length()),
				m_textFormat,
				textRect,
				m_textBrush
			);
		}

		// Draw composition string if in IME composition mode
		if (m_isComposing && !m_compositionString.empty()) {
			renderTarget->DrawTextW(
				m_compositionString.c_str(),
				static_cast<UINT32>(m_compositionString.length()),
				m_textFormat,
				textRect,
				m_compositionBrush
			);
		}
	}

	void TextBox::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
		if (message == WM_LBUTTONDOWN) {
			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			if (PtInRectF(m_rect, pt)) {
				m_hasFocus = true;
				// Set IME context to this window
				HIMC hImc = ImmGetContext(m_parent->GetHwnd());
				if (hImc) {
					ImmSetOpenStatus(hImc, TRUE);
					ImmReleaseContext(m_parent->GetHwnd(), hImc);
				}
			}
			else {
				m_hasFocus = false;
			}
		}
		
	}

	void TextBox::OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
		
		case WM_CHAR:
			if (wParam == VK_BACK) {
				if (!m_text.empty()) {
					m_text.pop_back();
				}
			}
			else if (wParam >= 32) { // Accept any printable character
				m_text += static_cast<wchar_t>(wParam);

			}
			break;
		case WM_IME_STARTCOMPOSITION:
			m_isComposing = true;
			m_compositionString.clear();
			break;
		case WM_IME_COMPOSITION: {
			HIMC hImc = ImmGetContext(m_parent->GetHwnd());
			if (hImc) {
				// 设置输入法窗口位置（在控件下方）
				COMPOSITIONFORM cf = { 0 };
				cf.dwStyle = CFS_POINT;
				cf.ptCurrentPos.x = static_cast<LONG>(m_rect.left);
				cf.ptCurrentPos.y = static_cast<LONG>(m_rect.bottom);
				ImmSetCompositionWindow(hImc, &cf);

				// 原有处理逻辑...
				if (lParam & GCS_COMPSTR) {
					// 获取组合字符串
					LONG len = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);
					if (len > 0) {
						m_compositionString.resize(len / sizeof(wchar_t));
						ImmGetCompositionStringW(hImc, GCS_COMPSTR, &m_compositionString[0], len);
					}
				}

				HIMC hImc = ImmGetContext(m_parent->GetHwnd());
				if (hImc) {
					if (lParam & GCS_COMPSTR) {
						// Get composition string
						LONG len = ImmGetCompositionStringW(hImc, GCS_COMPSTR, nullptr, 0);
						if (len > 0) {
							m_compositionString.resize(len / sizeof(wchar_t));
							ImmGetCompositionStringW(hImc, GCS_COMPSTR, &m_compositionString[0], len);
						}
					}
					if (lParam & GCS_RESULTSTR) {
						// Commit the final composition string
						LONG len = ImmGetCompositionStringW(hImc, GCS_RESULTSTR, nullptr, 0);
						if (len > 0) {
							std::wstring resultStr;
							resultStr.resize(len / sizeof(wchar_t));
							ImmGetCompositionStringW(hImc, GCS_RESULTSTR, &resultStr[0], len);
							m_text += resultStr;
						}
						m_isComposing = false;
						m_compositionString.clear();
					}
					ImmReleaseContext(m_parent->GetHwnd(), hImc);
				}
				ImmReleaseContext(m_parent->GetHwnd(), hImc);
			}
			break;
		}

		case WM_IME_ENDCOMPOSITION:
			m_isComposing = false;
			m_compositionString.clear();
			break;
		}
	}

	void TextBox::SetText(const std::wstring& text) {
		m_text = text;
	}
}