#pragma once

class Vector3
{
public:
	float x;
	float y;
	float z;

public:
	Vector3(float x, float y, float z) : x(x), y(y), z(z)
	{}

	Vector3() : x(0), y(0), z(0)
	{}
};