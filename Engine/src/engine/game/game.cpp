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
	// Pre frame
	UpdateDeltaTime();
	UpdateInput();

	// Frame
	MovePlayer();

	RaycastResult result = Raycast(camPos, vec3(0, 0, 1).rotate(vec3(camRot)), 7);
	selectedBlock = result.block;
	hasSelectedBlock = result.hit;

	if (hasSelectedBlock)
	{
		if (GetButtonPressed(BUTTON_PLACE))
		{
			SetBlock(selectedBlock + result.normal, 1);
		}
		if (GetButtonPressed(BUTTON_BREAK))
		{
			SetBlock(selectedBlock, 0);
		}
	}

	// Post frame
	EndOfFrameButtons();
	VK_Frame();
}

API void EndGame()
{
	VK_End();

	DestroyMainWindow();
	DestroyConsole();
}
