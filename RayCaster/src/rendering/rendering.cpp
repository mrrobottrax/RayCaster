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
	camera.RenderFrame(viewColorBuffer);
}

constexpr BYTE map[] = {
	1, 1, 1, 1, 1,
	1, 0, 1, 0, 0,
	1, 0, 0, 0, 0,
	1, 0, 0, 0, 0,
	0, 0, 0, 0, 1,
};

ScanLine GetScanLine(Vector3& position, float angle)
{
	// find dist to wall


	ScanLine line = {
		50,
		300
	};

	return line;
}
