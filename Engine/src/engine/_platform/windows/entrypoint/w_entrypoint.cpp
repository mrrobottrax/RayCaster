#include "pch.h"
#include "w_entrypoint.h"

#include "w_instance.h"
#include "game/game.h"
#include <_platform/windows/window/w_window.h>
#include <_platform/windows/input/w_input.h>

void W_EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	W_Instance::hInstance = hInstance;
	W_Instance::nCmdShow = nCmdShow;

	try
	{
		StartGame();

		// run the message loop.
		MSG msg = {};
		while (true)
		{
			// todo: stop processing messages after they take too long

			while (PeekMessage(&msg, 0, 0, WM_INPUT - 1, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
				{
					goto out_of_loop;
				}
			}

			while (PeekMessage(&msg, 0, WM_INPUT + 1, (UINT)-1, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			GameFrame();
		}

		out_of_loop:

		EndGame();
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "FATAL ERROR!", MB_ICONERROR);

		::exit(EXIT_FAILURE);
	}

	::exit(EXIT_SUCCESS);
}