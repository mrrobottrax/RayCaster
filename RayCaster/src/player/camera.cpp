#include "pch.h"
#include "Camera.h"
#include <rendering/Rendering.h>
#include <common/Math.h>

constexpr float increment = 2.f / width;

void DrawColumn(const int column, RColor* buffer, const Vector2& position, const Vector2& camForwards, const float yaw)
{
	const float offset = -1 + column * increment;
	const float angle = atanf(offset) + yaw;

	ScanLine scan = GetScanLine(position, angle, camForwards);

	// draw column
	const float u = scan.northSouth ? fmodf(scan.hitPos.x, 1) : fmodf(scan.hitPos.y, 1);
	for (int row = 0; row < height; ++row)
	{
		const int i = row * width + column;

		const float v = (row - scan.wallEnd) / static_cast<float>(scan.wallStart - scan.wallEnd);

		if (row >= scan.wallStart && row < scan.wallEnd)
		{
			if (scan.wallType == 255)
				buffer[i] = { 0, 0, 255 };
			else
				buffer[i] = { static_cast<unsigned char>(u * 255), static_cast<unsigned char>(v * 255), 255 };
		}
		else
		{
			if (row > height / 2)
			{
				buffer[i] = { 200, 200, 200 };
			}
			else
			{
				buffer[i] = { 255, 204, 102 };
			}
		}
	}
}

void Camera::RenderFrame(RColor* buffer)
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));

	Vector2 camForwards(
		cos(yaw),
		sin(yaw)
	);

	// collect and drawscan lines
	for (int column = 0; column < width; ++column)
	{
		DrawColumn(column, buffer, position, camForwards, yaw);
	}
}
