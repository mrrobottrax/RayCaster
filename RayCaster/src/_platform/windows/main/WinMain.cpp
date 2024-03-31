#include <pch.h>
#include "WinMain.h"

#include <game/Game.h>
#include <_platform/windows/window/W_Window.h>

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	W_Instance::hInstance = hInstance;
	W_MainWindow::nCmdShow = nCmdShow;

	GameInit();

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
		}
	}

	GameEnd();

	::exit(EXIT_SUCCESS);
}