#include <pch.h>
#include "Rendering.h"
#include <game/Game.h>

void InitRendering()
{
	viewColorBuffer = new RColor[width * height];
	memset(viewColorBuffer, 0, GetColorDataSize());
}

void CloseRendering()
{
	delete[] viewColorBuffer;
}

void RenderFrame(Camera& camera)
{
	camera.RenderFrame(viewColorBuffer, width, height);
}