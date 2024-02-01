#include <pch.h>
#include "rendering.h"
#include <game/game.h>

void InitRendering()
{
	colorData = new RColor[width * height];
	memset(colorData, 0, GetColorDataSize());
}

void CloseRendering()
{
	delete[] colorData;
}

void RenderFrame()
{
	localPlayer.RenderFrame();
}