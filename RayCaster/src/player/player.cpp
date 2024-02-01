#include "pch.h"
#include "player.h"
#include <rendering/rendering.h>

void Player::RenderFrame()
{
	memset(colorData, 255, GetColorDataSize());
}