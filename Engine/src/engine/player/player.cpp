#include "pch.h"
#include "player.h"

#include <input/button.h>
#include <time/time.h>
#include <input/mouse.h>
#include <world/world.h>

vec3 camPos{ 0, 0, 0 };
vec2 camRot{ 0, 0 };
uvec3 selectedBlock{ 0, 0, 0 };
bool hasSelectedBlock = false;

constexpr vec3 playerExtents{ 0.5f, 0.5f, 0.5f };

static float CastPlayerBox(const vec3& start, const vec3& direction, const float distance)
{
	// Create AABB touching all possible colliding voxels
	const vec3 end = start + direction * distance;
	const vec3 min = vec3::min(start, end) - playerExtents;
	const vec3 max = vec3::max(start, end) + playerExtents;

	// Create AABB of all possible colliding voxels
	const uvec3 startCoord = uvec3(min);
	const uvec3 endCoord = uvec3(max);

	// Create player AABB
	const vec3 playerMin = start - playerExtents;
	const vec3 playerMax = start + playerExtents;

	const vec3 slope(std::abs(1 / direction.x), std::abs(1 / direction.y), std::abs(1 / direction.z)); // divide by 0 should be okay on most platforms, im lazy

	// Loop of potentially colliding voxels and clip t
	float t = distance;
	// todo: check at each level if a collision is possible
	for (uint32_t x = startCoord.x; x <= endCoord.x; ++x)
	{
		for (uint32_t y = startCoord.y; y <= endCoord.y; ++y)
		{
			for (uint32_t z = startCoord.z; z <= endCoord.z; ++z)
			{
				if (GetBlock(uvec3(x, y, z)) == 0)
				{
					continue;
				}

				bool collision = false;
				for (int i = 0; i < 3; ++i)
				{
					const uint32_t& _grid = i == 0 ? x : (i == 1 ? y : z);
					const float& _slope = slope[i];

					const float& _playerMin = playerMin[i];
					const float& _playerMax = playerMax[i];
					const float& _dir = direction[i];

					const int i1 = (i + 1) % 3;
					const float& _playerMin1 = playerMin[i1];
					const float& _playerMax1 = playerMax[i1];
					const float& _dir1 = direction[i1];

					const int i2 = (i + 2) % 3;
					const float& _playerMin2 = playerMin[i2];
					const float& _playerMax2 = playerMax[i2];
					const float& _dir2 = direction[i2];

					const float dist = (_dir > 0 ? _grid - _playerMax : _playerMin - _grid) - 0.0001f;

					if (dist < 0)
					{
						// We're stuck!
						collision = true;
						t = 0;
						break;
					}

					const float _t = dist * _slope; // t till we hit the plane

					// Check if we can possibly hit the collision plane
					if (_t <= t)
					{
						// Check if we hit the face
						float max1 = _playerMax1 + _dir1 * _t;
						float min1 = _playerMin1 + _dir1 * _t;

						float max2 = _playerMax2 + _dir2 * _t;
						float min2 = _playerMin2 + _dir2 * _t;

						bool hit1 = (max1 > y) && (min1 < y + 1);
						bool hit2 = (max2 > z) && (min2 < z + 1);

						if (hit1 && hit2)
						{
							// Collision!
							collision = true;
							t = _t; // make sure you check _t < t first
							break;
						}
					}
				}
			}
		}
	}

	return t;
}

void MovePlayer()
{
	constexpr float moveSpeed = 3;
	constexpr float rotSpeed = 2;
	constexpr double sensitivity = 3 * (6.28319 / 16384.0);

	if (GetButtonDown(BUTTON_LOOK_LEFT)) { camRot.y += rotSpeed * Time::deltaTime; }
	if (GetButtonDown(BUTTON_LOOK_RIGHT)) { camRot.y -= rotSpeed * Time::deltaTime; }
	if (GetButtonDown(BUTTON_LOOK_UP)) { camRot.x += rotSpeed * Time::deltaTime; }
	if (GetButtonDown(BUTTON_LOOK_DOWN)) { camRot.x -= rotSpeed * Time::deltaTime; }

	double x, y;
	GetMouseDeltaD(&x, &y);
	camRot.y -= (float)(x * sensitivity);
	camRot.x -= (float)(y * sensitivity);

	vec3 moveDir{};
	if (GetButtonDown(BUTTON_FORWARD)) { moveDir.z++; }
	if (GetButtonDown(BUTTON_BACK)) { moveDir.z--; }
	if (GetButtonDown(BUTTON_LEFT)) { moveDir.x--; }
	if (GetButtonDown(BUTTON_RIGHT)) { moveDir.x++; }
	moveDir = moveDir.rotate(camRot.x, camRot.y);

	if (GetButtonDown(BUTTON_UP)) { moveDir.y++; }
	if (GetButtonDown(BUTTON_DOWN)) { moveDir.y--; }

	moveDir = moveDir.normalize();

	// Check collisions
	float dist = CastPlayerBox(camPos, moveDir, moveSpeed * Time::deltaTime);

	camPos += moveDir * dist;

	if (camPos.x > chunkSize) camPos.x = chunkSize;
	if (camPos.y > chunkSize) camPos.y = chunkSize;
	if (camPos.z > chunkSize) camPos.z = chunkSize;

	if (camPos.x < 0) camPos.x = 0;
	if (camPos.y < 0) camPos.y = 0;
	if (camPos.z < 0) camPos.z = 0;
}