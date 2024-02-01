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

	//while (!contents)
	//{
	//	// get dist to next wall
	//	Vector2Int next;
	//	Vector2 nextOffset;

	//	next.x = gridPos.x + (ray.dir.x > 0 ? 1 : 0);
	//	next.y = gridPos.y + (ray.dir.y > 0 ? 1 : 0);

	//	nextOffset.x = (ray.dir.x > 0 ? 1 - std::fmodf(pos.x, 1) : std::fmodf(pos.x, 1));
	//	nextOffset.y = (ray.dir.y > 0 ? 1 - std::fmodf(pos.y, 1) : std::fmodf(pos.y, 1));

	//	// find closest wall
	//	if (abs(nextOffset.x / ray.dir.x) > abs(nextOffset.y / ray.dir.y))
	//	{
	//		// move x
	//		const float dist = abs(nextOffset.y / ray.dir.y);
	//		totalDist.y += dist;
	//		pos.y = static_cast<float>(next.y);
	//		pos.x += dist * ray.dir.x;

	//		gridPos.y += nextOffset.y > 0 ? 1 : -1;
	//	}
	//	else
	//	{
	//		// move y
	//		const float dist = abs(nextOffset.x / ray.dir.x);
	//		totalDist.x += dist;
	//		pos.x = static_cast<float>(next.x);
	//		pos.y += dist * ray.dir.y;

	//		gridPos.x += nextOffset.x > 0 ? 1 : -1;
	//	}

	//	// check if wall is solid
	//	contents = GetWallContents(gridPos);
	//}

	float distX = ray.dir.x > 0 ? 1 - std::fmodf(pos.x, 1) : std::fmodf(pos.x, 1);
	distX /= abs(ray.dir.x);
	float distY = ray.dir.y > 0 ? 1 - std::fmodf(pos.y, 1) : std::fmodf(pos.y, 1);
	distY /= abs(ray.dir.y);

	if (distX < distY)
	{
		totalDist.x += distX * ray.dir.x;
		totalDist.y += distX * ray.dir.y;
	}
	else
	{
		totalDist.x += distY * ray.dir.x;
		totalDist.y += distY * ray.dir.y;
	}

	return RaycastResult{
		totalDist.Magnitude()
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