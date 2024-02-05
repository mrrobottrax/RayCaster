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

Ray GetPixelRay(const int column, const int row, const Camera& camera,
	const Vector3& forwards, const Vector3& right, const Vector3& up)
{
	float rightScale = column / static_cast<float>(viewWidth) * 2.f - 1;
	float upScale = -row / static_cast<float>(viewHeight) * 2.f + 1;

	Vector3 dir = forwards + right * rightScale + up * upScale;
	dir.Normalize();

	return {
		camera.position,
		dir
	};
}