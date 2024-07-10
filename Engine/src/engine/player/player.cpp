#include "pch.h"
#include "player.h"

#include <input/button.h>
#include <time/time.h>
#include <input/mouse.h>
#include <world/world.h>

vec3 camPos{ 0, 0, 0 };
vec3 prevCamPos{ 0, 0, 0 };
vec3 nextCamPos{ 0, 0, 0 };
vec3 position{ 0, 0, 0 };
vec2 camRot{ 0, 0 };
uvec3 selectedBlock{ 0, 0, 0 };
bool hasSelectedBlock = false;
vec3 selectedBlockNormal{ 0, 0, 0 };
vec3 velocity{ 0, 0, 0 };

constexpr float accel = 40;
constexpr float frict = 25;
constexpr float moveSpeed = 3.5f;
constexpr float airAccel = 20;
constexpr float airSpeed = 0.5f;
constexpr float gravity = 10;
bool grounded = false;

constexpr vec3 playerExtents{ 0.5f, 1, 0.5f };

struct CastResult
{
	bool collision;
	float dist;
	float fract;
	vec3 normal;
};

static CastResult CastPlayerBox(const vec3& start, const vec3& direction, const float distance)
{
	if (distance == 0)
	{
		CastResult result{};
		result.collision = false;
		result.fract = 1;
		result.dist = distance;
		return result;
	}

	const vec3 center{ start.x, start.y + playerExtents.y, start.z };

	// Create AABB touching all possible colliding voxels
	const vec3 end = center + direction * distance;
	const vec3 min = vec3::min(center, end) - playerExtents;
	const vec3 max = vec3::max(center, end) + playerExtents;

	// Create AABB of all possible colliding voxels
	const uvec3 startCoord = uvec3(min);
	const uvec3 endCoord = uvec3(max);

	// Create player AABB
	const vec3 playerMin = center - playerExtents;
	const vec3 playerMax = center + playerExtents;

	const vec3 slope(std::abs(1 / direction.x), std::abs(1 / direction.y), std::abs(1 / direction.z)); // divide by 0 should be okay on most platforms, im lazy

	// Loop of potentially colliding voxels and clip t
	// todo: check at each level if a collision is possible
	float t = distance;
	bool collision = false;
	vec3 normal;
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

				// Check if we actually collide with this block
				for (int i = 2; i >= 0; --i) // check each face
				{
					const float& _playerMin = playerMin[i];
					const float& _playerMax = playerMax[i];
					const float& _dir = direction[i];
					const uint32_t& _grid = i == 0 ? x : (i == 1 ? y : z);
					const float& _slope = slope[i];

					if (_dir == 0)
					{
						continue;
					}

					const int i1 = (i + 1) % 3;
					const float& _playerMin1 = playerMin[i1];
					const float& _playerMax1 = playerMax[i1];
					const float& _dir1 = direction[i1];
					const uint32_t& _grid1 = i1 == 0 ? x : (i1 == 1 ? y : z);

					const int i2 = (i + 2) % 3;
					const float& _playerMin2 = playerMin[i2];
					const float& _playerMax2 = playerMax[i2];
					const float& _dir2 = direction[i2];
					const uint32_t& _grid2 = i2 == 0 ? x : (i2 == 1 ? y : z);

					const float dist = _dir > 0 ? _grid - _playerMax : _playerMin - _grid - 1;

					if (dist < 0)
					{
						// Stuck in this plane, don't collide with it
						continue;
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

						bool hit1 = (max1 > _grid1) && (min1 < _grid1 + 1);
						bool hit2 = (max2 > _grid2) && (min2 < _grid2 + 1);

						if (hit1 && hit2)
						{
							// Collision!
							t = _t; // make sure you check _t < t first
							collision = true;
							normal = vec3(i == 0 ? 1.f : 0.f, i == 1 ? 1.f : 0.f, i == 2 ? 1.f : 0.f) * (_dir < 0 ? 1.f : -1.f);
							break;
						}
					}
				}
			}
		}
	}

	CastResult result{};
	result.collision = collision;
	result.normal = normal;
	result.dist = t;
	result.fract = t / distance;
	return result;
}

static bool GroundCheck()
{
	CastResult result = CastPlayerBox(position, vec3{ 0, -1, 0 }, 0.01f);
	if (result.collision && result.normal.y > 0.7f)
	{
		return true;
	}

	return false;
}

static void MovePlayer()
{
	vec3 moveDir{};
	if (GetButtonDown(BUTTON_FORWARD)) { moveDir.z++; }
	if (GetButtonDown(BUTTON_BACK)) { moveDir.z--; }
	if (GetButtonDown(BUTTON_LEFT)) { moveDir.x--; }
	if (GetButtonDown(BUTTON_RIGHT)) { moveDir.x++; }
	moveDir = moveDir.rotate(0, camRot.y);
	moveDir = moveDir.normalize();

	if (GetButtonDown(BUTTON_UP)) { velocity.y = 10; }

	grounded = GroundCheck();

	if (grounded)
	{
		velocity.y = 0;

		// Friction
		float speed = velocity.magnitude();
		if (speed != 0)
		{
			float newSpeed = std::max(speed - frict * Time::tickDeltaTime, 0.f);
			velocity *= newSpeed / speed;
		}

		// Acceleration
		velocity += moveDir * accel * Time::tickDeltaTime;
		speed = velocity.magnitude();
		if (speed > moveSpeed)
		{
			velocity *= moveSpeed / speed;
		}
	}
	else
	{
		// Acceleration
		float add = airAccel * Time::tickDeltaTime;
		float dot = vec3::dot(moveDir, velocity);
		if (dot + add > airSpeed)
		{
			add = std::max(airSpeed - dot, 0.f);
		}
		velocity += moveDir * add;

		velocity.y -= gravity * Time::tickDeltaTime;
	}

	// Check collisions
	float time = Time::tickDeltaTime;
	while (time > 0)
	{
		float mag = velocity.magnitude();
		vec3 dir = velocity.normalize();

		CastResult result = CastPlayerBox(position, dir, mag * time);
		position += dir * result.dist;

		if (result.collision)
		{
			velocity -= result.normal * vec3::dot(result.normal, velocity);
			time -= Time::tickDeltaTime * result.fract;
		}
		else
		{
			break;
		}
	}

	if (position.x > chunkSize) position.x = chunkSize;
	if (position.y > chunkSize) position.y = chunkSize;
	if (position.z > chunkSize) position.z = chunkSize;

	if (position.x < 0) position.x = 0;
	if (position.y < 0) { position.y = 0; velocity.y = 0; }
	if (position.z < 0) position.z = 0;
}

void PlayerFrame()
{
	// Rotate camera
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

	// Find selected block
	RaycastResult result = Raycast(camPos, vec3(0, 0, 1).rotate(vec3(camRot)), 7);
	selectedBlock = result.block;
	hasSelectedBlock = result.hit;
	selectedBlockNormal = result.normal;

	camPos = vec3::lerp(prevCamPos, nextCamPos, Time::tickFraction);
}

void PlayerTick()
{
	MovePlayer();
	prevCamPos = nextCamPos;
	nextCamPos = position + vec3(0, 1.75f, 0);

	if (hasSelectedBlock)
	{
		if (GetButtonPressedTick(BUTTON_PLACE))
		{
			SetBlock(selectedBlock + selectedBlockNormal, 1);
		}
		if (GetButtonPressedTick(BUTTON_BREAK))
		{
			SetBlock(selectedBlock, 0);
		}
	}
}