#include <pch.h>
#include "RWindows.h"
#include <rendering/Rendering.h>
#include <input/Buttons.h>

using namespace Gdiplus;

const wchar_t CLASS_NAME[] = L"Main Window Class";

HWND InitWindow(WNDPROC WindowProc, HINSTANCE hInstance)
{
	// set up window class
	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	RECT rect = {0, 0, 800, 600 };
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

void EndWindow(HWND hwnd, HINSTANCE hInstance)
{
	DestroyWindow(hwnd);
	UnregisterClass(CLASS_NAME, hInstance);
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

RKeys TranslateKey(WPARAM wParam)
{
	// std::cout << wParam << "\n";

	if (wParam >= IN_LETTERS_START && wParam <= IN_LETTERS_END)
	{
		return static_cast<RKeys>(wParam);
	}

	switch (wParam)
	{
		case VK_LEFT:
			return IN_ARROW_LEFT;
		case VK_RIGHT:
			return IN_ARROW_RIGHT;
		case VK_UP:
			return IN_ARROW_UP;
		case VK_DOWN:
			return IN_ARROW_DOWN;
		case VK_SPACE:
			return IN_KEY_SPACE;
		case VK_SHIFT:
			return IN_KEY_SHIFT;
		default:
			break;
	}

	return IN_BAD_KEY;
}

void KeyDown(WPARAM wParam)
{
	RKeys button = TranslateKey(wParam);
	keys[button] = true;
}

void KeyUp(WPARAM wParam)
{
	RKeys button = TranslateKey(wParam);
	keys[button] = false;
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