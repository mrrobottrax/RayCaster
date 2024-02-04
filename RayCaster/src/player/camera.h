#pragma once

#include <rendering/RColor.h>
#include <common/Vector.h>

class Camera
{
public:
	Vector2 position;
	float yaw = 0;

public:
	void RenderFrame(RColor* pBuffer);
};