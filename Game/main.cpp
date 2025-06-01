#include "KroubleUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // ��������
        KroubleUI::Window mainWindow(hInstance, L"KroubleUI ʾ��", 800, 600);
		auto button = new KroubleUI::Button(&mainWindow, D2D1::RectF(100, 100, 300, 150), L"�����");
		
        button->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::LightBlue));

		auto textBlock = new KroubleUI::TextBlock(&mainWindow, D2D1::RectF(100, 200, 300, 250), L"����һ���ı���");
		textBlock->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textBlock->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		textBlock->SetWordWrap(true);
		textBlock->SetTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		textBlock->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::Yellow));
        button->SetOnClickHandler([textBlock]() {
			textBlock->SetText(L"��ť������ˣ�");
            });
        auto button2 = new KroubleUI::Button(&mainWindow, D2D1::RectF(200, 100, 400, 250), L"�����");
		button2->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::LightGreen));
		button2->SetOnClickHandler([textBlock]() {
			textBlock->SetText(L"��ť2������ˣ�");
			});
		// ��ӿؼ�������
		mainWindow.AddControl(button2);
		mainWindow.AddControl(button);
		mainWindow.AddControl(textBlock);
        // ������Ϣѭ��
        mainWindow.RunMessageLoop();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "����", MB_ICONERROR);
        return 1;
    }

    return 0;
}