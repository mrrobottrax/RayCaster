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

RaycastResult CastRay(const Ray& ray)
{
	Vector3 pos(ray.start);
	Vector2Int gridPos = GetGridPos(pos);
	WallType contents = 0;

	int lastType = 0;
	while (!contents)
	{
		float distX, distY, distZ;

		// find distances to next line
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

		distZ = ray.dir.z > 0 ? 1 - std::fmodf(pos.z, 1) : std::fmodf(pos.z, 1);
		distZ /= abs(ray.dir.z);

		if (distX < distY && distX < distZ)
		{
			gridPos.x += ray.dir.x > 0 ? 1 : -1;

			pos.x += distX * ray.dir.x;
			pos.y += distX * ray.dir.y;
			pos.z += distX * ray.dir.z;

			lastType = 1;
		}
		else if (distY < distX && distY < distZ)
		{
			gridPos.y += ray.dir.y > 0 ? 1 : -1;

			pos.x += distY * ray.dir.x;
			pos.y += distY * ray.dir.y;
			pos.z += distY * ray.dir.z;

			lastType = 2;
		}
		else
		{
			pos.x += distZ * ray.dir.x;
			pos.y += distZ * ray.dir.y;
			pos.z += distZ * ray.dir.z;

			contents = 0;
			break;
		}

		// check if wall is solid
		contents = GetGridType(gridPos);
	}

	Vector3 normal;
	if (contents == 0)
	{
		normal = Vector3(0, 0, ray.dir.z > 0 ? -1.f : 1.f);
	}
	else if (lastType == 1)
	{
		normal = Vector3(1, 0, 0);
	}
	else if (lastType == 2)
	{
		normal = Vector3(0, 1, 0);
	}

	return RaycastResult{
		pos,
		normal,
		contents,
	};
}

Vector2Int GetGridPos(const Vector3& position)
{
	return GetGridPos(Vector2(position.x, position.y));
}

Vector2Int GetGridPos(const Vector2& position)
{
	return {
		static_cast<int>(position.x),
		static_cast<int>(position.y)
	};
}

WallType GetGridType(const Vector2Int& wallPosition)
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