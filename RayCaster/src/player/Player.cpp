#include "pch.h"
#include "player.h"
#include <input/buttons.h>
#include <common/Math.h>

void Player::Update()
{
	if (buttons[IN_ARROW_LEFT])
	{
		camera.yaw -= 0.01f;
	}

	if (buttons[IN_ARROW_RIGHT])
	{
		camera.yaw += 0.01f;
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

	moveVector.Rotate(camera.yaw);

	moveVector *= 0.01f;
	position += moveVector;

	camera.yaw = fmodf(camera.yaw, pi2);

	camera.position = position;
}
