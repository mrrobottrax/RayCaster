#include "pch.h"
#include "Camera.h"
#include <rendering/Rendering.h>
#include <common/Math.h>
#include <map/Map.h>

void Camera::RenderFrame(RColor* buffer) const
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));

	const Vector3 forwards(
		cos(yaw),
		sin(yaw),
		0
	);
	const Vector3 right(
		sin(yaw),
		-cos(yaw),
		0
	);
	const Vector3 up(
		0,
		0,
		1
	);

	// trace rays
	int i = 0;
	for (int row = 0; row < viewHeight; ++row)
	{
		for (int column = 0; column < viewWidth; ++column)
		{
			Ray ray = GetPixelRay(column, row, *this, forwards, right, up);

			constexpr int sampleCount = 2;
			Vector3 color(0, 0, 0);
			for (int j = 0; j < sampleCount; ++j)
			{
				color += TracePath(ray, 0) / sampleCount;
			}

			buffer[i] = {
				static_cast<unsigned char>(color.z * 255),
				static_cast<unsigned char>(color.y * 255),
				static_cast<unsigned char>(color.x * 255)
			};

			// RaycastResult result = CastRay(ray);
			/*buffer[i] = {
				static_cast<unsigned char>(result.normal.x * 255),
				static_cast<unsigned char>(result.normal.y * 255),
				static_cast<unsigned char>(result.normal.z * 255)
			};*/

			++i;
		}
	}
}
