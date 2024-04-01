#include "pch.h"
#include "Game.h"

#include <_wrappers/window/WindowWrapper.h>
#include <_wrappers/console/ConsoleWrapper.h>
#include <_platform/vulkan/VK_Graphics.h>

void GameInit()
{
	CreateConsole();
	CreateMainWindow();

	VK_Init();
}

void GameFrame()
{
}

void GameEnd()
{
	DestroyMainWindow();
	DestroyConsole();
}
