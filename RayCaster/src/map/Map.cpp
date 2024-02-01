#include "pch.h"
#include "Map.h"

constexpr int mapWidth = 5;
constexpr int mapHeight = 5;
constexpr WallType map[] = {
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 1, 1,
	0, 0, 0, 0, 0,
	1, 1, 1, 0, 0,
};

RaycastResult CastRay(Ray& ray)
{
	Vector2 pos(ray.pos);
	Vector2Int gridPos = GetGridPos(pos);
	WallType contents = 0;

	int lastType = 0;
	while (!contents)
	{
		float distX, distY;

		if (lastType == 1)
		{
			distX = 1 / abs(ray.dir.x);
		}
		else
		{
			distX = ray.dir.x > 0 ? 1 - std::fmodf(pos.x, 1) : std::fmodf(pos.x, 1);
			distX /= abs(ray.dir.x);
		}

		if (lastType == 2)
		{
			distY = 1 / abs(ray.dir.y);
		}
		else
		{
			distY = ray.dir.y > 0 ? 1 - std::fmodf(pos.y, 1) : std::fmodf(pos.y, 1);
			distY /= abs(ray.dir.y);
		}

		if (distX < distY)
		{
			gridPos.x += ray.dir.x > 0 ? 1 : -1;

			pos.x += distX * ray.dir.x;
			pos.y += distX * ray.dir.y;

			lastType = 1;
		}
		else
		{
			gridPos.y += ray.dir.y > 0 ? 1 : -1;

			pos.x += distY * ray.dir.x;
			pos.y += distY * ray.dir.y;

			lastType = 2;
		}

		// check if wall is solid
		contents = GetWallType(gridPos);
	}

	return RaycastResult{
		pos,
		contents
	};
}

Vector2Int GetGridPos(Vector2& position)
{
	return {
		static_cast<int>(position.x),
		static_cast<int>(position.y)
	};
}

WallType GetWallType(Vector2Int& wallPosition)
{
	// check if out of bounds
	if (wallPosition.x < 0 || wallPosition.x >= mapWidth
		|| wallPosition.y < 0 || wallPosition.y >= mapHeight)
	{
		return 255;
	}

	const int index = mapWidth * wallPosition.y + wallPosition.x;
	return map[index];
}