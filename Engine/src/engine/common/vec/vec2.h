#pragma once

template <typename T>
class vec2_base
{
public:
	union
	{
		struct { T x; T y; };
	};
};

typedef vec2_base<float> vec2;
typedef vec2_base<uint32_t> uvec2;