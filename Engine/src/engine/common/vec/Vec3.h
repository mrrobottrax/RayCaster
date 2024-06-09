#pragma once

template <typename T>
class vec3_base
{
public:
	union
	{
		struct { T x; T y; T z; };
		struct { T r; T g; T b; };
	};
};

typedef vec3_base<float> vec3;