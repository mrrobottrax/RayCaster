#include "pch.h"
#include "Game.h"

#include <_wrappers/window/WindowWrapper.h>
#include <_wrappers/console/ConsoleWrapper.h>

void GameInit()
{
	CreateConsole();
	CreateMainWindow();
}

void GameFrame()
{
}

void GameEnd()
{
	DestroyMainWindow();
	DestroyConsole();
}
