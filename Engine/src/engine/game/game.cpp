#include "pch.h"
#include "Game.h"

#include "_wrappers/window/window_wrapper.h"
#include "_wrappers/console/console_wrapper.h"
#include "_wrappers/graphics/graphics_wrapper.h"

#include "setup/callbacks.h"

API void StartGame()
{
	CreateConsole();
	CreateMainWindow();

	StartGraphics();

	PostInitCallback();
}

API void GameFrame()
{
}

API void EndGame()
{
	EndGraphics();

	DestroyMainWindow();
	DestroyConsole();
}
