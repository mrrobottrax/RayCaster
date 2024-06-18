#pragma once

constexpr int chunkSize = 64;

inline uint8_t chunkData[chunkSize * chunkSize * chunkSize];
inline bool chunkOutOfDate = false;

void InitWorld();
void SetBlock(ivec3 location, uint8_t type);
uint8_t GetBlock(ivec3 location);

struct Ray
{
	vec3 origin;
	vec3 direction;
};

struct RaycastResult
{
	bool hit;
	vec3 surfacePos;
	ivec3 block;
};

RaycastResult Raycast(Ray ray, float maxDist = INFINITY);