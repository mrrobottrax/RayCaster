#include "pch.h"
#include "game.h"
#include <rendering/rendering.h>

void InitGame()
{
	InitRendering();
}

void CloseGame()
{
	CloseRendering();
}

void GameFrame()
{
	RenderFrame();
}
