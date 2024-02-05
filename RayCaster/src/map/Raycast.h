#pragma once
#include <common/Vector.h>
#include "Walltype.h"

struct Ray
{
	Vector3 start;
	Vector3 dir;
};

struct RaycastResult
{
	Vector3 point;
	Vector3 normal;
	WallType wallType;
};