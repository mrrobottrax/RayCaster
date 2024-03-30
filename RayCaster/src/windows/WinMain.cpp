#include <pch.h>

#include "RWindows.h"
#include <game/Game.h>
#include <rendering/software/SoftwareRendering.h>
#include "Direct3DGraphics.h"

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

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	InitGame();

	if (!InitD3D11(hwnd))
	{
		_wassert(_CRT_WIDE("Failed to init d3d11!"), _CRT_WIDE(__FILE__), (unsigned)(__LINE__));
	}

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
			GameFrame();

		#ifdef USE_SOFTWARE_RENDERER
			DrawSoftwareFrameD3D11();
		#else
			DrawFrameD3D11();
		#endif // USE_SOFTWARE_RENDER
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