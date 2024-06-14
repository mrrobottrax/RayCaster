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

	void rotate(float pitch, float yaw, float roll)
	{
		vec3_base<T> v{};

		const T cP = cos(pitch);
		const T cY = cos(yaw);
		const T cR = cos(roll);

		const T sP = sin(pitch);
		const T sY = sin(yaw);
		const T sR = sin(roll);

		v.x = x * cR * cY + y * (cP * sR + sP * cR * sY) + z * (sP * sR - cP * cR * sY);
		v.y = -x * sR * cY + y * (cP * cR - sP * sR * sY) + z * (sP * cR - cP * sR * sY);
		v.z = x * sY - y * sP * cY + z * cP * cY;

		*this = v;
	}

	void normalize()
	{
		T sqrMagnitude = x * x + y * y + z * z;

		if (sqrMagnitude == 0)
		{
			return;
		}

		T magnitude = sqrt(sqrMagnitude);

		x /= magnitude;
		y /= magnitude;
		z /= magnitude;
	}

	// ~~~~~~~~~~~~~~~~~~~ OPERATORS ~~~~~~~~~~~~

	// Addition
	constexpr vec3_base<T>& operator+ (vec3_base<T>&& v) const
	{
		v.x += this->x;
		v.y += this->y;
		v.z += this->z;
		return v;
	}

	constexpr vec3_base<T> operator+ (const vec3_base<T>& v) const
	{
		return std::move(vec3_base<T>{this->x + v.x, this->y + v.y, this->z + v.z});
	}

	constexpr vec3_base<T>& operator+= (const vec3_base<T>& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	// Multiplication
	constexpr vec3_base<T> operator* (const float f) const
	{
		return vec3_base<T>{this->x * f, this->y * f, this->z * f};
	}

	constexpr vec3_base<T>& operator*= (const float f)
	{
		this->x *= f;
		this->y *= f;
		this->z *= f;
		return *this;
	}

};

typedef vec3_base<float> vec3;