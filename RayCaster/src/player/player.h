#pragma once
#include "Camera.h"

class Player
{
public:
	Camera camera;
	Vector2 position;

public:
	Player(Vector2 position) : position(position)
	{}

	void Update();
};