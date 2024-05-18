#include "pch.h"
#include "entrypoint.h"

#include "win_instance.h"
#include "game/game.h"

void W_EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	W_Instance::hInstance = hInstance;
	W_Instance::nCmdShow = nCmdShow;

	try {
		StartGame();

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

		EndGame();
	}
	catch (const std::exception& e)
	{
		MessageBoxA(nullptr, e.what(), "FATAL ERROR!", MB_ICONERROR);

		::exit(EXIT_FAILURE);
	}

	::exit(EXIT_SUCCESS);
}