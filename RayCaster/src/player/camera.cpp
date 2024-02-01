#include "pch.h"
#include "Camera.h"
#include <rendering/Rendering.h>
#include <common/Math.h>

constexpr float increment = 2.f / width;

void Camera::RenderFrame(RColor* buffer)
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));

	Vector2 camForwards(
		cos(yaw),
		sin(yaw)
	);

	// collect scan lines
	ScanLine scanLines[width]{};
	for (int scan = 0; scan < width; ++scan)
	{
		float offset = -1 + scan * increment;
		float angle = atanf(offset) + yaw;

		scanLines[scan] = GetScanLine(position, angle, camForwards);
	}

	// draw scans
	for (int row = 0; row < height; ++row)
	{
		for (int column = 0; column < width; ++column)
		{
			const int i = row * width + column;
			const ScanLine& scan = scanLines[column];

			if (row >= scan.wallStart && row < scan.wallEnd)
			{
				buffer[i] = {255, 255, 255};
			}
			else
			{
				buffer[i] = {0, 0, 0};
			}
		}
	}
}
