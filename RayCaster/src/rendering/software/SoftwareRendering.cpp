#include <pch.h>
#include "SoftwareRendering.h"
#include <game/Game.h>
#include <map/Map.h>
#include <common/Math.h>

void InitSoftwareRendering()
{
	softwareRenderTarget = new RColor[renderWidth * renderHeight];
	memset(softwareRenderTarget, 0, GetColorDataSize());
}

void CloseSoftwareRendering()
{
	delete[] softwareRenderTarget;
}

void RenderFrameSoftware(const Camera& camera, RColor* buffer, const int width, const int height)
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));

	const Vector3 forwards(
		cos(camera.yaw),
		sin(camera.yaw),
		0
	);
	const Vector3 right(
		sin(camera.yaw),
		-cos(camera.yaw),
		0
	);
	const Vector3 up(
		0,
		0,
		1
	);

	// trace rays
	int i = 0;
	for (int row = 0; row < height; ++row)
	{
		for (int column = 0; column < width; ++column)
		{
			Ray ray = GetPixelRay(column, row, camera.position, forwards, right, up);

			const RaycastResult result = CastRay(ray);

			Vector3 color;

			switch (result.wallType)
			{
				case 0:
					color.x = 1;
					color.y = 1;
					color.z = 1;
					break;

				case 1:
					color.x = 1;
					color.y = 0;
					color.z = 0;
					break;
			}

			buffer[i] = {
				color.x, color.y, color.z, 1
			};

			++i;
		}
	}
}

Ray GetPixelRay(const int column, const int row, const Vector3& position,
	const Vector3& forwards, const Vector3& right, const Vector3& up)
{
	float rightScale = column / static_cast<float>(renderWidth) * 2.f - 1;
	float upScale = -row / static_cast<float>(renderHeight) * 2.f + 1;

	Vector3 dir = forwards + right * rightScale + up * upScale;
	dir.Normalize();

	return {
		position,
		dir
	};
}