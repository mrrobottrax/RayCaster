#include "pch.h"
#include "Game.h"

#include <_wrappers/window/WindowWrapper.h>
#include <_wrappers/console/ConsoleWrapper.h>
#include <_wrappers/graphics/GraphicsWrapper.h>

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
