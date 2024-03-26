#pragma once
#include "rendering/Camera.h"

class Player
{
public:
	Camera camera;

private:
	Vector3 position;

public:
	Player(Vector3 position) : position(position)
	{}

	void Update();
	void TryMove(const Vector3& velocity);
};