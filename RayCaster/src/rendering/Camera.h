#pragma once

#include <rendering/RColor.h>
#include <common/Vector.h>

class Camera
{
public:
	Vector3 position;
	float pitch = 0;
	float yaw = 0;
};