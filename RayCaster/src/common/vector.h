#pragma once

class Vector
{

};

class Vector3 : Vector
{
public:
	float x;
	float y;
	float z;

public:
	Vector3() : x(0), y(0), z(0)
	{}

	Vector3(float x, float y, float z) : x(x), y(y), z(z)
	{}
};

class Vector2 : Vector
{
public:
	float x;
	float y;

public:
	Vector2() : x(0), y(0)
	{}

	Vector2(float x, float y) : x(x), y(y)
	{}
};