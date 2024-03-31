#include "pch.h"
#include "Game.h"

#include <_platform/_wrappers/window/WindowWrapper.h>
#include <_platform/_wrappers/console/ConsoleWrapper.h>

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
