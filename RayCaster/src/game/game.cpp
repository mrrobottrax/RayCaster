#include "pch.h"
#include "game.h"
#include <rendering/Rendering.h>
#include "Time.h"

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
	Time::UpdateTime();
	localPlayer.Update();
}
