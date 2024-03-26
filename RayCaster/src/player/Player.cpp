#include "pch.h"
#include "player.h"
#include <input/buttons.h>
#include <common/Math.h>
#include <map/Map.h>

void Player::Update()
{
	if (keys[IN_ARROW_LEFT])
	{
		camera.yaw += 0.01f;
	}

	if (keys[IN_ARROW_RIGHT])
	{
		camera.yaw -= 0.01f;
	}

	if (keys[IN_ARROW_UP])
	{
		camera.pitch += 0.01f;
	}

	if (keys[IN_ARROW_DOWN])
	{
		camera.pitch -= 0.01f;
	}

	Vector3 moveVector(0, 0, 0);

	if (keys['W'])
	{
		++moveVector.y;
	}

	if (keys['S'])
	{
		--moveVector.y;
	}

	if (keys['D'])
	{
		++moveVector.x;
	}

	if (keys['A'])
	{
		--moveVector.x;
	}

	if (keys[IN_KEY_SPACE])
	{
		++moveVector.z;
	}

	if (keys[IN_KEY_SHIFT])
	{
		--moveVector.z;
	}

	camera.yaw = fmodf(camera.yaw, two_pi);
	moveVector.RotateYaw(camera.yaw - half_pi);
	moveVector *= 0.01f;

	TryMove({moveVector.x, moveVector.y, moveVector.z});

	camera.position = Vector3(position.x, position.y, position.z);
}

void Player::TryMove(const Vector3& velocity)
{
	Vector3 newVel(velocity);
	//Vector3Int blockPos = GetGridPos(position);

	//constexpr float playerSize = 0.2f;

	//// get dist to wall
	//const float distX = (velocity.x > 0 ? 1 - fmodf(position.x, 1) : fmodf(position.x, 1)) - playerSize;
	//const float distY = (velocity.y > 0 ? 1 - fmodf(position.y, 1) : fmodf(position.y, 1)) - playerSize;

	//constexpr float backOff = 0.1f;

	//// check if this move will hit the wall
	//if (abs(velocity.x) >= distX)
	//{
	//	// check if solid wall
	//	if (GetGridType(blockPos + Vector2Int((velocity.x > 0 ? 1 : -1), 0)) > 0)
	//	{
	//		newVel.x = 0;
	//	}
	//}
	//if (abs(velocity.y) >= distY)
	//{
	//	// check if solid wall
	//	if (GetGridType(blockPos + Vector2Int(0, (velocity.y > 0 ? 1 : -1))) > 0)
	//	{
	//		newVel.y = 0;
	//	}
	//}

	position += newVel;
}