#include <pch.h>
#include "Rendering.h"
#include <game/Game.h>
#include <map/Map.h>

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

ScanLine GetScanLine(Vector3& position, float angle, Vector2& cameraForwards)
{
	constexpr float wallHeight = 100;

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

	// get dist along camera normal (no fisheye)
	const float dist = Vector2::Dot(cast.point, cameraForwards);

	int halfSize = static_cast<int>(wallHeight / dist);
	int middle = height / 2;

	ScanLine line = {
		middle - halfSize,
		middle + halfSize
	};

	return line;
}
