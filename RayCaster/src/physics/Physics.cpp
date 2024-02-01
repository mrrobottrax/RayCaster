#include "pch.h"
#include "Physics.h"

constexpr int mapWidth = 5;
constexpr int mapHeight = 5;
constexpr BYTE map[] = {
	1, 1, 1, 1, 1,
	1, 0, 1, 0, 0,
	1, 0, 0, 0, 0,
	1, 0, 0, 0, 0,
	0, 0, 0, 0, 1,
};

RaycastResult CastRay(Ray& ray)
{
	// get offset into cell
	Vector2 currentPos(ray.pos);

	return RaycastResult{
		10
	};
}