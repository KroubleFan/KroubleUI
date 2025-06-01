#include "KroubleUI.h"

namespace KroubleUI {

	Window::Window(HINSTANCE hInstance, const std::wstring& title, int width, int height) : m_hwnd(nullptr), m_d2dFactory(nullptr), m_dwriteFactory(nullptr), m_renderTarget(nullptr) {

		// 注册窗口类
		WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = sizeof(LONG_PTR);
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpszClassName = L"KroubleUIWindow";
		wcex.hbrBackground = nullptr;

		if (!RegisterClassExW(&wcex)) {
			throw std::runtime_error("Failed to register window class");
		}

		// 创建窗口
		m_hwnd = CreateWindowW(
			L"KroubleUIWindow",
			title.c_str(),
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			width, height,
			nullptr,
			nullptr,
			hInstance,
			this
		);

		if (!m_hwnd) {
			throw std::runtime_error("Failed to create window");
		}

		// 初始化Direct2D和DirectWrite
		InitializeDirect2D();

		ShowWindow(m_hwnd, SW_SHOW);
		UpdateWindow(m_hwnd);
	}

	void Window::InitializeDirect2D() {
		// 创建D2D工厂
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

		HRESULT hr = D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory),
			&options,
			reinterpret_cast<void**>(&m_d2dFactory)
		);

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create D2D factory");
		}

		// 创建DirectWrite工厂
		hr = DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&m_dwriteFactory)
		);

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create DirectWrite factory");
		}

		// 创建渲染目标
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		hr = m_d2dFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(m_hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
			&m_renderTarget
		);

		if (FAILED(hr)) {
			throw std::runtime_error("Failed to create render target");
		}

	}

	void Window::Render() {
		m_renderTarget->BeginDraw();
		m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::LightGray));

		for (auto& control : m_controls) {
			control->Draw(m_renderTarget);
		}

		HRESULT hr = m_renderTarget->EndDraw();
		if (hr == D2DERR_RECREATE_TARGET) {
			DiscardGraphicsResources();
			InitializeDirect2D();
		}
	}

	void Window::AddControl(Control* control) {
		auto tmp = std::unique_ptr<Control>(control);
		m_controls.push_back(std::move(tmp));
		Render();
	}

	void Window::DiscardGraphicsResources() {
		SafeRelease(&m_renderTarget);
	}

	LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Window* pThis = nullptr;

		if (message == WM_NCCREATE) {
			CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = reinterpret_cast<Window*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
		}
		else {
			pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (pThis) {
			switch (message) {
			case WM_PAINT:
				pThis->Render();
				ValidateRect(hwnd, nullptr);
				return DefWindowProc(hwnd, message, wParam, lParam);

			case WM_SIZE: {
				if (pThis->m_renderTarget) {
					RECT rc;
					GetClientRect(hwnd, &rc);
					pThis->m_renderTarget->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
					pThis->Render();
				}
				return 0;
			}

			case WM_DISPLAYCHANGE:
				pThis->Render();
				return 0;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MOUSEMOVE:
				pThis->OnMouseEvent(message, wParam, lParam);
				return 0;
			case WM_IME_STARTCOMPOSITION:
			case WM_IME_COMPOSITION:
			case WM_IME_ENDCOMPOSITION:
				pThis->OnKeyboardEvent(message, wParam, lParam);
				return 0;
			case WM_IME_SETCONTEXT:
				// 确保显示输入法窗口
				if (wParam == TRUE) {
					lParam |= ISC_SHOWUICOMPOSITIONWINDOW;
				}
				break;
			case WM_CHAR:
			case WM_KEYDOWN:
			case WM_KEYUP:
				pThis->OnKeyboardEvent(message, wParam, lParam);
				return 0;

			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			}
		}

		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	void Window::OnMouseEvent(UINT message, WPARAM wParam, LPARAM lParam) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		bool eventHandled = false;

		// 逆序遍历控件（从最上层到最下层）
		for (auto it = m_controls.rbegin(); it != m_controls.rend(); ++it) {
			auto& control = *it;

			if (!control->IsVisible()) continue;

			bool isInside = control->HitTest(static_cast<float>(pt.x), static_cast<float>(pt.y));

			switch (message) {
			case WM_MOUSEMOVE:
				if (!eventHandled) {
					// 只让最上层符合条件的控件处理悬停状态
					control->OnMouseEvent(isInside ? WM_MOUSEMOVE : WM_MOUSELEAVE, wParam, lParam);
					if (isInside) eventHandled = true;
				}
				else {
					// 其他控件强制设为非悬停状态
					control->OnMouseEvent(WM_MOUSELEAVE, wParam, lParam);
				}
				break;

			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
				if (isInside && !eventHandled) {
					control->OnMouseEvent(message, wParam, lParam);
					eventHandled = true;
				}
				break;

			case WM_MOUSELEAVE:
				// 通知所有控件鼠标离开
				control->OnMouseEvent(WM_MOUSELEAVE, wParam, lParam);
				break;
			}
		}
	}

	void  Window::OnKeyboardEvent(UINT message, WPARAM wParam, LPARAM lParam) {
		for (auto& control : m_controls) {
			control->OnKeyboardEvent(message, wParam, lParam);
		}
		Render();
	}

	void Window::RunMessageLoop() {
		MSG msg = { 0 };

		while (msg.message != WM_QUIT) {
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else {
				Render();
			}
		}

	}

} // namespace KroubleUI