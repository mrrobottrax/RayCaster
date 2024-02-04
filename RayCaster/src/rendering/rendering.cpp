#include <pch.h>
#include "Rendering.h"
#include <game/Game.h>
#include <map/Map.h>

void InitRendering()
{
	viewColorBuffer = new RColor[viewWidth * viewHeight];
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

void CopyViewBuffer()
{

}

ScanLine GetScanLine(const Vector2& position, const float angle, const Vector2& cameraForwards)
{
	constexpr int wallHeight = viewWidth / 4;

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
	const float dist = Vector2::Dot(cast.point - position, cameraForwards);

	const int halfSize = static_cast<int>(wallHeight / dist);
	const int middle = viewHeight / 2;

	return {
		middle - halfSize,
		middle + halfSize,
		cast.wallType,
		cast.northSouth,
		cast.point
	};
}
