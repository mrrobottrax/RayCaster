#include "pch.h"
#include "game.h"
#include <rendering/Rendering.h>

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
	RenderFrame(localPlayer.camera);
}
