#pragma once
#include <common/Vector.h>

struct Ray
{
	Vector2 dir;
	Vector2 pos;
};

struct RaycastResult
{
	float dist;
};