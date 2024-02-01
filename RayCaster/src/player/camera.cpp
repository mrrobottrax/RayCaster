#include "pch.h"
#include "Camera.h"
#include <rendering/Rendering.h>

constexpr BYTE map[] = {
1, 1, 1, 1, 1,
1, 0, 1, 0, 0,
1, 0, 0, 0, 0,
1, 0, 0, 0, 0,
0, 0, 0, 0, 0,
};

constexpr float halfFov = 45;
float halfX = atanf(halfFov);
float increment = 2 * halfX * width;

void Camera::RenderFrame(RColor* buffer, int width, int height)
{
	// memset(buffer, 255, static_cast<size_t>(width) * height * sizeof(RColor));
	for (int scanLine = 0; scanLine < width; ++scanLine)
	{
		float x = -halfFov + scanLine * increment;
	}
}
