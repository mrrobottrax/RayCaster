#include "pch.h"
#include "W_Window.h"

#include "_platform/windows/entrypoint/win_instance.h"

constexpr wchar_t CLASS_NAME[] = L"MainWindow";

LRESULT CALLBACK W_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void W_CreateMainWindow()
{
	// set up window class
	WNDCLASS wc = {};

	wc.lpfnWndProc = W_WindowProc;
	wc.hInstance = W_Instance::hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	RECT rect = { 0, 0, 800, 600 };
	AdjustWindowRectEx(&rect, dwStyle, FALSE, 0);

	W_Window::hWnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Game",                        // Window text
		dwStyle,						// Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,

		NULL,       // Parent window    
		NULL,		// Menu
		W_Instance::hInstance,  // Instance handle
		NULL        // Additional application data
	);

	ShowWindow(W_Window::hWnd, W_Instance::nCmdShow);
	UpdateWindow(W_Window::hWnd);
}

void W_DestroyMainWindow()
{
	DestroyWindow(W_Window::hWnd);
	UnregisterClass(CLASS_NAME, W_Instance::hInstance);
}

LRESULT CALLBACK W_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		//case WM_PAINT:
		//	OnPaint(hwnd);
		//	return 0;

	case WM_KEYDOWN:
		// KeyDown(wParam);
		break;

	case WM_KEYUP:
		// KeyUp(wParam);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void W_GetMainWindowClientSize(uint32_t* pWidth, uint32_t* pHeight)
{
	RECT rect;
	GetClientRect(W_Window::hWnd, &rect);

	*pWidth = rect.right;
	*pHeight = rect.bottom;
}