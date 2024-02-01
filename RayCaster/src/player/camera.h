#pragma once

#include <common/vector.h>
#include <wtypes.h>

class Camera
{
private:
	Vector3 position;
	float yaw = 0;

public:
	void RenderFrame(BYTE* pBytes, int width, int height);
};