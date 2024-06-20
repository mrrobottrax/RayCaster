#pragma once

constexpr int chunkSize = 64;

inline uint8_t chunkData[chunkSize * chunkSize * chunkSize];
inline bool chunkOutOfDate = false;

void InitWorld();
void SetBlock(ivec3 location, uint8_t type);
uint8_t GetBlock(ivec3 location);

struct RaycastResult
{
	bool hit = false;
	vec3 hitPos;
	ivec3 block;
	vec3 normal;
	float dist;
};

RaycastResult Raycast(const vec3& origin, const vec3& direction, float maxDist = INFINITY);