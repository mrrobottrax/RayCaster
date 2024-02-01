#include "pch.h"
#include "player.h"
#include <input/buttons.h>

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

	camera.yaw = fmodf(camera.yaw, 360);
}
