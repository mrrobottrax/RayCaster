#include <pch.h>

#include "RWindows.h"
#include <game/Game.h>
#include <game/Time.h>
#include <rendering/Rendering.h>
#include "Direct3DGraphics.h"

using namespace Gdiplus;

VOID OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);

	Graphics graphics(hdc);
	Bitmap bitmap(viewWidth, viewHeight, static_cast<int>(sizeof(RColor)) * viewWidth, PixelFormat24bppRGB, reinterpret_cast<BYTE*>(viewColorBuffer));

	graphics.DrawImage(&bitmap, 0, 0, viewWidth * renderScale, viewHeight * renderScale);

	EndPaint(hwnd, &ps);
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

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	InitD3D11(hwnd);

	InitGame();

	// run the message loop.
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			// fps limit
			// if (Time::TimeSinceLastUpdate() > 0.01f)
			{
				GameFrame();
				// InvalidateRgn(hwnd, NULL, FALSE);
				// UpdateWindow(hwnd);
				DrawFrameD3D11();
			}
		}
	}

	CloseGame();

	EndD3D11();

	std::cout << "CLOSING GAME\n";
	CloseConsole();

	EndWindow(hwnd, hInstance);

	::exit(EXIT_SUCCESS);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
		KeyDown(wParam);
		break;

	case WM_KEYUP:
		KeyUp(wParam);
		break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}