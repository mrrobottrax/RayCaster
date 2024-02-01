#include "pch.h"
#include "Map.h"

constexpr int mapWidth = 5;
constexpr int mapHeight = 5;
constexpr WallContents map[] = {
	1, 1, 1, 1, 1,
	1, 0, 1, 0, 0,
	1, 0, 0, 0, 0,
	1, 0, 0, 0, 0,
	0, 0, 0, 0, 1,
};

RaycastResult CastRay(Ray& ray)
{
	Vector2 pos(ray.pos);
	Vector2Int gridPos = GetGridPos(pos);
	WallContents contents = 0;
	Vector2 totalDist;

	while (!contents)
	{
		float distX = ray.dir.x > 0 ? 1 - std::fmodf(pos.x, 1) : std::fmodf(pos.x, 1);
		distX /= abs(ray.dir.x);
		float distY = ray.dir.y > 0 ? 1 - std::fmodf(pos.y, 1) : std::fmodf(pos.y, 1);
		distY /= abs(ray.dir.y);

		if (distX < distY)
		{
			gridPos.x += ray.dir.x > 0 ? 1 : -1;

			pos.x += distX * ray.dir.x;
			pos.y += distX * ray.dir.y;
		}
		else
		{
			gridPos.y += ray.dir.y > 0 ? 1 : -1;

			pos.x += distY * ray.dir.x;
			pos.y += distY * ray.dir.y;
		}

		// check if wall is solid
		contents = GetWallContents(gridPos);
	}

	return RaycastResult{
		pos
	};
}

Vector2Int GetGridPos(Vector2& position)
{
	return {
		static_cast<int>(position.x),
		static_cast<int>(position.y)
	};
}

WallContents GetWallContents(Vector2Int& wallPosition)
{
	// check if out of bounds
	if (wallPosition.x < 0 || wallPosition.x >= mapWidth
		|| wallPosition.y < 0 || wallPosition.y >= mapHeight)
	{
		return 1;
	}

	const int index = mapHeight - mapHeight * wallPosition.y + wallPosition.x;
	return map[index];
}