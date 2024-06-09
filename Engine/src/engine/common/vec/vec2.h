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