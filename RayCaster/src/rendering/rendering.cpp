#include <pch.h>
#include "Rendering.h"
#include <game/Game.h>
#include <physics/Physics.h>

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

ScanLine GetScanLine(Vector3& position, float angle)
{
	constexpr float wallHeight = 200;

	// find dist to wall
	Ray ray{
		{
			cos(angle),
			sin(angle)
		},
		{
			position.x,
			position.y
		}
	};

	RaycastResult cast = CastRay(ray);

	int halfSize = static_cast<int>(wallHeight / cast.dist);
	int middle = height / 2;

	ScanLine line = {
		middle - halfSize,
		middle + halfSize
	};

	return line;
}
