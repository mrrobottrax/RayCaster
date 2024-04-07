#include "pch.h"
#include "Game.h"

#include <_wrappers/window/window_wrapper.h>
#include <_wrappers/console/console_wrapper.h>
#include <_wrappers/graphics/graphics_wrapper.h>

void GameInit()
{
	CreateConsole();
	CreateMainWindow();

	InitGraphics();
}

void GameFrame()
{
}

void GameEnd()
{
	EndGraphics();

	DestroyMainWindow();
	DestroyConsole();
}
