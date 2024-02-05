#pragma once
#include "vector.h"

constexpr float d2r = 0.0174533f;
constexpr float pi = 3.14159265358979323846f;
constexpr float pi2 = pi * 2;

inline constexpr float Deg2Rad(float d)
{
	return d2r * d;
}

inline constexpr float Rad2Deg(float r)
{
	return r / d2r;
}

Vector3 RandomUnitVector();
Vector3 RandomHemisphereVector(const Vector3& normal);