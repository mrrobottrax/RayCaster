#include "pch.h"
#include "game.h"

#include "_wrappers/window/window_wrapper.h"
#include "_wrappers/console/console_wrapper.h"

#include "setup/callbacks.h"
#include <graphics/vk.h>
#include <time/time.h>
#include <input/mouse.h>
#include <input/input.h>
#include <player/player.h>

API void StartGame()
{
	CreateConsole();
	CreateMainWindow();

	VK_Start();

	PostInitCallback();
}

API void GameFrame()
{
	UpdateDeltaTime();
	UpdateInput();

	MovePlayer();

	VK_Frame();
}

API void EndGame()
{
	VK_End();

	DestroyMainWindow();
	DestroyConsole();
}
