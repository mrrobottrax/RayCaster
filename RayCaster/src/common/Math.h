#pragma once

constexpr float d2r = 0.0174533f;

inline constexpr float Deg2Rad(float d)
{
	return d2r * d;
}

inline constexpr float Rad2Deg(float r)
{
	return r / d2r;
}