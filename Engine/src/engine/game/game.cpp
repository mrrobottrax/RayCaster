#include "pch.h"
#include "Game.h"

#include "_wrappers/window/window_wrapper.h"
#include "_wrappers/console/console_wrapper.h"

#include "setup/callbacks.h"
#include <graphics/vk.h>

API void StartGame()
{
	CreateConsole();
	CreateMainWindow();

	VK_Start();

	PostInitCallback();
}

API void GameFrame()
{
	VK_Frame();
}

API void EndGame()
{
	VK_End();

	DestroyMainWindow();
	DestroyConsole();
}
