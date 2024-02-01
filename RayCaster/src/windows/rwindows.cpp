#include <pch.h>
#include "rwindows.h"
#include <rendering/rendering.h>

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