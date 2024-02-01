#pragma once

template <typename T>
class Vector
{
	virtual float Magnitude() const = 0;
};

template <typename T>
class Vector3Base : Vector<T>
{
public:
	T x;
	T y;
	T z;

public:
	Vector3Base() : x(0), y(0), z(0)
	{}

	Vector3Base(T x, T y, T z) : x(x), y(y), z(z)
	{}

public:
	float Magnitude() const override
	{
		return sqrt(static_cast<float>(x * x + y * y + z * z));
	}
};

template <typename T>
class Vector2Base : Vector<T>
{
public:
	T x;
	T y;

public:
	Vector2Base() : x(0), y(0)
	{}

	Vector2Base(T x, T y) : x(x), y(y)
	{}

public:
	float Magnitude() const override
	{
		return sqrtf(static_cast<float>(x * x + y * y));
	}
};

class Vector3 : public Vector3Base<float>
{
public:
	Vector3() : Vector3Base()
	{}

	Vector3(float x, float y, float z) : Vector3Base(x, y, z)
	{}
};

class Vector2 : public Vector2Base<float>
{
public:
	Vector2() : Vector2Base()
	{}

	Vector2(float x, float y) : Vector2Base(x, y)
	{}
};

class Vector2Int : public Vector2Base<int>
{
public:
	Vector2Int() : Vector2Base()
	{}

	Vector2Int(int x, int y) : Vector2Base(x, y)
	{}
};