#include "KroubleUI.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // 创建窗口
        KroubleUI::Window mainWindow(hInstance, L"KroubleUI 示例", 800, 600);
		auto button = new KroubleUI::Button(&mainWindow, D2D1::RectF(100, 100, 300, 150), L"点击我");
		
        button->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::LightBlue));

		auto textBlock = new KroubleUI::TextBlock(&mainWindow, D2D1::RectF(100, 200, 300, 250), L"这是一个文本块");
		textBlock->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textBlock->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		textBlock->SetWordWrap(true);
		textBlock->SetTextColor(D2D1::ColorF(D2D1::ColorF::Black));
		textBlock->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::Yellow));
        button->SetOnClickHandler([textBlock]() {
			textBlock->SetText(L"按钮被点击了！");
            });
        auto button2 = new KroubleUI::Button(&mainWindow, D2D1::RectF(200, 100, 400, 250), L"点击我");
		button2->SetBackgroundColor(D2D1::ColorF(D2D1::ColorF::LightGreen));
		button2->SetOnClickHandler([textBlock]() {
			textBlock->SetText(L"按钮2被点击了！");
			});
		// 添加控件到窗口
		mainWindow.AddControl(button2);
		mainWindow.AddControl(button);
		mainWindow.AddControl(textBlock);
        // 运行消息循环
        mainWindow.RunMessageLoop();
    }
    catch (const std::exception& e) {
        MessageBoxA(nullptr, e.what(), "错误", MB_ICONERROR);
        return 1;
    }

    return 0;
}