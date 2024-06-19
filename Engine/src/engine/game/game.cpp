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

API void StartGame()
{
	CreateConsole();
	CreateMainWindow();

	VK_Start();

	InitWorld();

	PostInitCallback();
}

API void GameFrame()
{
	UpdateDeltaTime();
	UpdateInput();

	MovePlayer();

	RaycastResult result = Raycast(camPos, vec3(0, 0, 1).rotate(vec3(camRot)));
	selectedBlock = result.block;

	if (GetButtonDown(BUTTON_PLACE))
	{
		SetBlock(selectedBlock, 1);
	}

	if (GetButtonDown(BUTTON_BREAK))
	{
		SetBlock(selectedBlock, 0);
	}

	VK_Frame();
}

API void EndGame()
{
	VK_End();

	DestroyMainWindow();
	DestroyConsole();
}
