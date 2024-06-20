#include "pch.h"
#include "world.h"

void InitWorld()
{
	for (int i = 0; i < chunkSize * chunkSize * chunkSize; ++i)
	{
		float r = (float)rand() / RAND_MAX;

		if (r > 0.95)
			chunkData[i] = 1;
		else
			chunkData[i] = 0;
	}

	chunkOutOfDate = true;
}

static uint32_t GetBlockIndex(ivec3 location)
{
	return location.x + location.y * chunkSize + location.z * chunkSize * chunkSize;
}

void SetBlock(ivec3 location, uint8_t type)
{
	if (location.x < 0 || location.y < 0 || location.z < 0) return;
	if (location.x > chunkSize || location.y > chunkSize || location.z > chunkSize) return;

	chunkOutOfDate = true;

	chunkData[GetBlockIndex(location)] = type;
}

uint8_t GetBlock(ivec3 location)
{
	return chunkData[GetBlockIndex(location)];
}

RaycastResult Raycast(const vec3& origin, const vec3& direction, float maxDist)
{
	// Calculate distances to next grid line
	float incX = 1.f / abs(direction.x);
	float incY = 1.f / abs(direction.y);
	float incZ = 1.f / abs(direction.z);

	float distX = (direction.x > 0 ? 1 - fmodf(origin.x, 1) : fmodf(origin.x, 1)) * incX;
	float distY = (direction.y > 0 ? 1 - fmodf(origin.y, 1) : fmodf(origin.y, 1)) * incY;
	float distZ = (direction.z > 0 ? 1 - fmodf(origin.z, 1) : fmodf(origin.z, 1)) * incZ;

	float totalDist = 0;
	bool hit = false;
	ivec3 gridPos = origin;
	vec3 normal{};
	for (int i = 0; i < 2048; ++i)
	{
		// Test dist
		if (totalDist > maxDist)
		{
			hit = false;
			break;
		}

		// Test bounds
		if (gridPos.x < 0 || gridPos.y < 0 || gridPos.z < 0 ||
			gridPos.x > chunkSize || gridPos.y > chunkSize || gridPos.z > chunkSize)
		{
			hit = false;
			break;
		}

		// Test if in a solid
		if (GetBlock(gridPos) > 0)
		{
			hit = true;
			break;
		}

		// Find smallest distances
		bool maskX = distX <= distY && distX <= distZ;
		bool maskY = distY <= distX && distY <= distZ;
		bool maskZ = distZ <= distX && distZ <= distY;

		if (maskX)
		{
			totalDist = distX;
			distX += incX;
			gridPos.x += direction.x > 0 ? 1 : -1;
			normal = vec3{ direction.x > 0 ? -1.f : 1.f, 0, 0 };
		}

		if (maskY)
		{
			totalDist = distY;
			distY += incY;
			gridPos.y += direction.y > 0 ? 1 : -1;
			normal = vec3{ 0, direction.y > 0 ? -1.f : 1.f, 0 };
		}

		if (maskZ)
		{
			totalDist = distZ;
			distZ += incZ;
			gridPos.z += direction.z > 0 ? 1 : -1;
			normal = vec3{ 0, 0, direction.z > 0 ? -1.f : 1.f };
		}
	}

	RaycastResult result{};
	result.hit = hit;
	result.block = gridPos;
	result.hitPos = origin + direction * totalDist;
	result.dist = totalDist;
	result.normal = normal;

	return result;
}