#include <pch.h>
#include "rendering.h"

void InitRendering()
{
	colorData = new BYTE[width * height * bytesPerPixel];
	memset(colorData, 0, static_cast<size_t>(width * height) * bytesPerPixel);
}

void CloseRendering()
{
	delete[] colorData;
}

void RenderFrame()
{
	memset(colorData, 3, static_cast<size_t>(width * height) * bytesPerPixel);
}