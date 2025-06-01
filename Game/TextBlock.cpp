#include "KroubleUI.h"

namespace KroubleUI {
	TextBlock::TextBlock(Window* parent, const D2D1_RECT_F& rect, const std::wstring& text)
		: Control(parent, rect), m_text(text), m_textBrush(nullptr), m_textFormat(nullptr),
		m_wordWrap(true), m_textAlignment(DWRITE_TEXT_ALIGNMENT_LEADING),
		m_paragraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR) {
		m_dwriteFactory = nullptr; // ��ʼ��Ϊnullptr
		Initialize(parent->GetRenderTarget(), parent->GetDWriteFactory());
	}

	void TextBlock::Initialize(ID2D1RenderTarget* renderTarget, IDWriteFactory* dwriteFactory) {
		// �����ı�����
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush);
		// �����������ʣ�Ĭ��͸����
		renderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0), &m_backgroundBrush);
		m_dwriteFactory = dwriteFactory;
		// �����ı���ʽ
		dwriteFactory->CreateTextFormat(
			L"Microsoft YaHei", // ֧�����ĵ�����
			nullptr,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			14.0f,
			L"zh-cn", // ����locale
			&m_textFormat
		);

		// ���ó�ʼ��ʽ
		UpdateTextFormat();

	}

	void TextBlock::Draw(ID2D1RenderTarget* renderTarget) {
		if (!m_visible || m_text.empty()) return;
		// ���Ʊ���
		if (m_backgroundBrush) {
			D2D1_COLOR_F bgColor = m_backgroundBrush->GetColor();
			if (bgColor.a > 0) {  // ֻ�з���ȫ͸��ʱ�Ż���
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

	// ��������ɫ���÷���
	void TextBlock::SetBackgroundColor(const D2D1_COLOR_F& color) {
		if (m_backgroundBrush) {
			m_backgroundBrush->SetColor(color);
		}
		else if (m_parent) {
			// �����δ��ʼ�����ӳٵ�Drawʱ����
			ID2D1RenderTarget* rt = m_parent->GetRenderTarget();
			if (rt) {
				rt->CreateSolidColorBrush(color, &m_backgroundBrush);
			}
		}
	}
}