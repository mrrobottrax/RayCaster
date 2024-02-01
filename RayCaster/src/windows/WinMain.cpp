#include <pch.h>

#include "RWindows.h"
#include <game/Game.h>
#include <rendering/Rendering.h>

using namespace Gdiplus;

VOID OnPaint(HDC hdc)
{
	GameFrame();

	Graphics graphics(hdc);
	Bitmap bitmap(width, height, static_cast<int>(sizeof(RColor)) * width, PixelFormat24bppRGB, reinterpret_cast<BYTE*>(viewColorBuffer));

	graphics.DrawImage(&bitmap, 0, 0);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	HWND hwnd = InitWindow(WindowProc, hInstance);

	if (hwnd == NULL)
	{
		return 0;
	}

	InitConsole();

	ULONG_PTR gdiplusToken = InitGdi();

	InitGame();

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// run the message loop.
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CloseGame();

	CloseGdi(gdiplusToken);
	CloseWindow();

	CloseConsole();

	::exit(EXIT_SUCCESS);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			OnPaint(hdc);

			EndPaint(hwnd, &ps);
		}
		return 0;

		case WM_KEYDOWN:
			KeyDown(wParam);
			break;

		case WM_KEYUP:
			KeyUp(wParam);
			break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}