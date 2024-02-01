#include "pch.h"
#include "Player.h"
#include <rendering/Rendering.h>

void Player::RenderFrame()
{
	memset(viewColorBuffer, 255, GetColorDataSize());
}