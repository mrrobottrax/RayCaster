#pragma once
#include "rendering/Camera.h"

class Player
{
public:
	Camera camera;

private:
	Vector2 position;

public:
	Player(Vector2 position) : position(position)
	{}

	void Update();
	void TryMove(const Vector2& velocity);
};