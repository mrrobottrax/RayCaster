#include "pch.h"
#include "Camera.h"
#include <rendering/Rendering.h>

constexpr float halfFov = 45;
float halfX = atanf(halfFov);
float increment = 2 * halfX * width;

void Camera::RenderFrame(RColor* buffer)
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));

	// collect scan lines
	ScanLine scanLines[width]{};
	for (int scan = 0; scan < width; ++scan)
	{
		float forwardsX = -halfFov + scan * increment;
		float angle = tanf(forwardsX);

		scanLines[scan] = GetScanLine(position, angle);
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
				buffer[i] = { 255, 255, 255 };
			}
		}
	}
}
