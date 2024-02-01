#include <pch.h>
#include "rwindows.h"
#include <rendering/rendering.h>
#include <input/buttons.h>

using namespace Gdiplus;

HWND InitWindow(WNDPROC WindowProc, HINSTANCE hInstance)
{
	// set up window class
	WNDCLASS wc = {};

	const wchar_t CLASS_NAME[] = L"Main Window Class";

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT rect = {0, 0, width, height};
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, FALSE, 0);

	DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Game",                        // Window text
		dwStyle,						// Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,

		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	return hwnd;
}

void CloseWindow()
{

}

ULONG_PTR InitGdi()
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	return gdiplusToken;
}

void CloseGdi(ULONG_PTR gdiplusToken)
{
	GdiplusShutdown(gdiplusToken);
}

RButton TranslateKey(WPARAM wParam)
{
	std::cout << wParam << "\n";

	return IN_BAD_KEY;
}

void KeyDown(WPARAM wParam)
{
	TranslateKey(wParam);
}

void KeyUp(WPARAM wParam)
{

}

void InitConsole()
{
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
}

void CloseConsole()
{
	// std::cin.get();
	FreeConsole();
}