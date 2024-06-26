#pragma once

constexpr int chunkSize = 64;

inline uint8_t chunkData[chunkSize * chunkSize * chunkSize];
inline bool chunkOutOfDate = false;

void InitWorld();
void SetBlock(uvec3 location, uint8_t type);
uint8_t GetBlock(uvec3 location);

struct RaycastResult
{
	bool hit = false;
	vec3 hitPos;
	uvec3 block;
	vec3 normal;
	float dist;
};

RaycastResult Raycast(const vec3& origin, const vec3& direction, float maxDist = INFINITY);