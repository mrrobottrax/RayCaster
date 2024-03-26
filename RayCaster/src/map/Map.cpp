#include "pch.h"
#include "Map.h"

constexpr WallType map[] = {
	0, 0, 0, 1, 0,
	0, 0, 0, 1, 0,
	0, 0, 0, 0, 0,
	1, 1, 0, 0, 0,
	0, 0, 0, 0, 0,

	0, 0, 0, 1, 0,
	0, 0, 0, 1, 0,
	0, 0, 0, 1, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 1, 0,
	0, 0, 0, 1, 1,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,

	0, 0, 0, 0, 0,
	0, 0, 0, 0, 0,
	0, 0, 1, 1, 0,
	0, 0, 1, 1, 0,
	0, 0, 0, 0, 0,
};

RaycastResult CastRay(const Ray& ray)
{
	Vector3 pos(ray.start);
	Vector3Int gridPos = GetGridPos(pos);
	WallType contents = 0;

	enum
	{
		none,
		x,
		y,
		z,
	} lastType = none;

	while (contents == 0)
	{
		float distX, distY, distZ;

		// find distances to next line
		if (lastType == x)
		{
			distX = 1 / abs(ray.dir.x);
		}
		else
		{
			distX = ray.dir.x > 0 ? 1 - std::fmodf(pos.x, 1) : std::fmodf(pos.x, 1);
			distX /= abs(ray.dir.x);
		}

		if (lastType == y)
		{
			distY = 1 / abs(ray.dir.y);
		}
		else
		{
			distY = ray.dir.y > 0 ? 1 - std::fmodf(pos.y, 1) : std::fmodf(pos.y, 1);
			distY /= abs(ray.dir.y);
		}

		if (lastType == z)
		{
			distZ = 1 / abs(ray.dir.z);
		}
		else
		{
			distZ = ray.dir.z > 0 ? 1 - std::fmodf(pos.z, 1) : std::fmodf(pos.z, 1);
			distZ /= abs(ray.dir.z);
		}

		if (distX < distY && distX < distZ)
		{
			// x next

			gridPos.x += ray.dir.x > 0 ? 1 : -1;

			pos.x += distX * ray.dir.x;
			pos.y += distX * ray.dir.y;
			pos.z += distX * ray.dir.z;

			lastType = x;
		}
		else if (distY < distX && distY < distZ)
		{
			// y next

			gridPos.y += ray.dir.y > 0 ? 1 : -1;

			pos.x += distY * ray.dir.x;
			pos.y += distY * ray.dir.y;
			pos.z += distY * ray.dir.z;

			lastType = y;
		}
		else
		{
			// z next

			gridPos.z += ray.dir.z > 0 ? 1 : -1;

			pos.x += distZ * ray.dir.x;
			pos.y += distZ * ray.dir.y;
			pos.z += distZ * ray.dir.z;

			lastType = z;
		}

		// check if wall is solid
		contents = GetGridType(gridPos);
	}

	Vector3 normal;
	if (lastType == 1)
	{
		normal = Vector3(ray.dir.x > 0 ? -1.f : 1.f, 0, 0);
	}
	else if (lastType == 2)
	{
		normal = Vector3(0, ray.dir.y > 0 ? -1.f : 1.f, 0);
	}

	return RaycastResult{
		pos,
		normal,
		contents,
	};
}

Vector3Int GetGridPos(const Vector3& position)
{
	return {
		static_cast<int>(position.x),
		static_cast<int>(position.y),
		static_cast<int>(position.z),
	};
}

WallType GetGridType(const Vector3Int& wallPosition)
{
	// check if out of bounds
	if (wallPosition.x < 0 || wallPosition.x >= mapWidth
		|| wallPosition.y < 0 || wallPosition.y >= mapDepth
		|| wallPosition.z < 0 || wallPosition.z >= mapHeight)
	{
		return -1;
	}

	const int index = wallPosition.x + (wallPosition.y * mapWidth) + wallPosition.z * mapWidth * mapDepth;
	return map[index];
}

const WallType* GetMapPointer()
{
	return map;
}