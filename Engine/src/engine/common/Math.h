#pragma once
constexpr float d2r = 0.0174533f;
constexpr float pi = 3.14159265358979323846f;
constexpr float two_pi = pi * 2;
constexpr float half_pi = pi / 2;

constexpr float Deg2Rad(float d)
{
	return d2r * d;
}

constexpr float Rad2Deg(float r)
{
	return r / d2r;
}