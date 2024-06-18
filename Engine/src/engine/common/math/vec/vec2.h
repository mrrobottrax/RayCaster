#pragma once

template <typename T>
class vec2_base
{
public:
	union
	{
		struct { T x; T y; };
	};

	// ~~~~~~~~~ OPERATORS ~~~~~~~~~~~

	// Conversion
	template <typename U>
	constexpr operator vec3_base<U>()
	{
		return vec3_base<U>(static_cast<U>(this->x), static_cast<U>(this->y), 0);
	}
};

typedef vec2_base<float> vec2;
typedef vec2_base<uint32_t> uvec2;