#include "pch.h"
#include "player.h"
#include <input/buttons.h>
#include <common/Math.h>
#include <map/Map.h>

void Player::Update()
{
	if (buttons[IN_ARROW_LEFT])
	{
		camera.yaw += 0.01f;
	}

	if (buttons[IN_ARROW_RIGHT])
	{
		camera.yaw -= 0.01f;
	}

	Vector2 moveVector(0, 0);

	if (buttons[IN_ARROW_UP])
	{
		++moveVector.x;
	}

	if (buttons[IN_ARROW_DOWN])
	{
		--moveVector.x;
	}

	camera.yaw = fmodf(camera.yaw, pi2);
	moveVector.Rotate(camera.yaw);
	moveVector *= 0.01f;

	TryMove(moveVector);

	camera.position = Vector3(position.x, position.y, 0.5);
}

void Player::TryMove(const Vector2& velocity)
{
	Vector2 newVel(velocity);
	Vector2Int blockPos = GetGridPos(position);

	constexpr float playerSize = 0.2f;

	// get dist to wall
	const float distX = (velocity.x > 0 ? 1 - fmodf(position.x, 1) : fmodf(position.x, 1)) - playerSize;
	const float distY = (velocity.y > 0 ? 1 - fmodf(position.y, 1) : fmodf(position.y, 1)) - playerSize;

	constexpr float backOff = 0.1f;

	// check if this move will hit the wall
	if (abs(velocity.x) >= distX)
	{
		// check if solid wall
		if (GetGridType(blockPos + Vector2Int((velocity.x > 0 ? 1 : -1), 0)))
		{
			newVel.x = 0;
		}
	}
	if (abs(velocity.y) >= distY)
	{
		// check if solid wall
		if (GetGridType(blockPos + Vector2Int(0, (velocity.y > 0 ? 1 : -1))))
		{
			newVel.y = 0;
		}
	}

	position += newVel;
}