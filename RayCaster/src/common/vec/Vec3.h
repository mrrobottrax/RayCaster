#pragma once

#include "VecBase.h"

typedef vec<3, float> vec3;
typedef vec<3, int> vec3int;

template <typename T>
struct vec3_base : public vec_base<3, T>
{
public:
	union { T x, r; };
	union { T y, g; };
	union { T z, b; };

	vec3_base() : x(0), y(0), z(0)
	{}

	vec3_base(T x, T  y, T z) : x(x), y(y), z(z)
	{}

	T SqrMagnitude() override
	{
		return x * x + y * y + z * z;
	}

	void Invert() override
	{
		x *= -1;
		y *= -1;
		z *= -1;
	}
};

template <typename T>
struct vec<3, T> : public vec3_base<T>
{
	vec() : vec3_base(0, 0, 0)
	{};

	vec(T x, T y, T z) : vec3_base<T>(x, y, z)
	{};
};

// Extra float stuff
template <>
struct vec<3, float> : public vec3_base<float>, public vec_base_float
{
public:
	vec() : vec3_base(0, 0, 0)
	{};

	vec(float x, float y, float z) : vec3_base(x, y, z)
	{};

	void Normalize() override
	{
		float m = Magnitude();

		x /= m;
		y /= m;
		z /= m;
	}
};