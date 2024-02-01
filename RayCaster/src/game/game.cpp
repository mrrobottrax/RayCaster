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
	localPlayer.Update();
	RenderFrame(localPlayer.camera);
}
