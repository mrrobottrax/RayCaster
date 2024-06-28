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
#include <world/world.h>
#include <input/button.h>

void GameTick()
{
	PlayerTick();

	EndOfTickButtons();
}

void StartGame()
{
	CreateConsole();
	CreateMainWindow();

	VK_Start();

	InitWorld();

	PostInitCallback();
}

void GameFrame()
{
	// Pre frame
	UpdateDeltaTime();
	UpdateInput();

	PlayerFrame();

	TryTick();

	// Post frame
	EndOfFrameButtons();
	VK_Frame();
}

void EndGame()
{
	VK_End();

	DestroyMainWindow();
	DestroyConsole();
}
