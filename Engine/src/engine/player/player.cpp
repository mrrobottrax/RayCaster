#include "pch.h"
#include "player.h"

#include <input/button.h>
#include <time/time.h>
#include <input/mouse.h>
#include <world/world.h>

vec3 camPos{ 0, 0, 0 };
vec2 camRot{ 0, 0 };
ivec3 selectedBlock{ 0, 0, 0 };
bool hasSelectedBlock = false;

void MovePlayer()
{
	constexpr float moveSeed = 3;
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

	vec3 moveVector{};
	if (GetButtonDown(BUTTON_FORWARD)) { moveVector.z++; }
	if (GetButtonDown(BUTTON_BACK)) { moveVector.z--; }
	if (GetButtonDown(BUTTON_LEFT)) { moveVector.x--; }
	if (GetButtonDown(BUTTON_RIGHT)) { moveVector.x++; }
	moveVector = moveVector.rotate(camRot.x, camRot.y);

	if (GetButtonDown(BUTTON_UP)) { moveVector.y++; }
	if (GetButtonDown(BUTTON_DOWN)) { moveVector.y--; }

	moveVector = moveVector.normalize();

	camPos = camPos + moveVector * moveSeed * Time::deltaTime;

	if (camPos.x > chunkSize) camPos.x = chunkSize;
	if (camPos.y > chunkSize) camPos.y = chunkSize;
	if (camPos.z > chunkSize) camPos.z = chunkSize;

	if (camPos.x < 0) camPos.x = 0;
	if (camPos.y < 0) camPos.y = 0;
	if (camPos.z < 0) camPos.z = 0;
}