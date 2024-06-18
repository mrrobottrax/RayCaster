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
	chunkOutOfDate = true;

	chunkData[GetBlockIndex(location)] = type;
}

uint8_t GetBlock(ivec3 location)
{
	return chunkData[GetBlockIndex(location)];
}

RaycastResult Raycast(Ray ray, float maxDist)
{
	return RaycastResult();
}