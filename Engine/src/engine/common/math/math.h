#pragma once

#include "mat/mat.h"
#include "vec/vec.h"

constexpr float d2r = 0.0174533f;
constexpr float pi = 3.14159265358979323846f;
constexpr float two_pi = pi * 2;
constexpr float half_pi = pi / 2;

constexpr float deg2Rad(float d)
{
	return d2r * d;
}

constexpr float rad2Deg(float r)
{
	return r / d2r;
}