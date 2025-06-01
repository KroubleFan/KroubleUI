#include "KroubleUI.h"

namespace KroubleUI {
	TextBlock::TextBlock(Window* parent, const D2D1_RECT_F& rect, const std::wstring& text)
		: Control(parent, rect), m_text(text), m_textBrush(nullptr), m_textFormat(nullptr),
		m_wordWrap(true), m_textAlignment(DWRITE_TEXT_ALIGNMENT_LEADING),
		m_paragraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR) {
		m_dwriteFactory = nullptr; // 初始化为nullptr
		Initialize(parent->GetRenderTarget(), parent->GetDWriteFactory());
	}

	void TextBlock::Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory) {
		// 创建文本画笔
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush);
		// 创建背景画笔（默认透明）
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0), &m_backgroundBrush);
		m_dwriteFactory = dwriteFactory;
		// 创建文本格式
		dwriteFactory->CreateTextFormat(
			L"Microsoft YaHei", // 支持中文的字体
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			14.0f,
			L"zh-cn", // 中文locale
			&m_textFormat
		);

		// 设置初始格式
		UpdateTextFormat();

	}

	void TextBlock::Draw(ID2D1RenderTarget* renderTarget) {
		if (!m_visible || m_text.empty()) return;
		// 绘制背景
		if (m_backgroundBrush) {
			D2D1_COLOR_F bgColor = m_backgroundBrush->GetColor();
			if (bgColor.a > 0) {  // 只有非完全透明时才绘制
				renderTarget->FillRectangle(m_rect, m_backgroundBrush);
			}
		}
		renderTarget->DrawTextW(
			m_text.c_str(),
			static_cast<UINT32>(m_text.length()),
			m_textFormat,
			m_rect,
			m_textBrush
		);
	}

	// 新增背景色设置方法
	void TextBlock::SetBackgroundColor(const D2D1_COLOR_F& color) {
		if (m_backgroundBrush) {
			m_backgroundBrush->SetColor(color);
		}
		else if (m_parent) {
			// 如果尚未初始化，延迟到Draw时处理
			ID2D1RenderTarget* rt = m_parent->GetRenderTarget();
			if (rt) {
				rt->CreateSolidColorBrush(color, &m_backgroundBrush);
			}
		}
	}
}