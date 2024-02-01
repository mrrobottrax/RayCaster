#pragma once
#include <common/Vector.h>
#include "Walltype.h"

struct Ray
{
	Vector2 dir;
	Vector2 pos;
};

struct RaycastResult
{
	Vector2 point;
	WallType wallType;
};