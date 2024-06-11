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

	void rotateYaw(float angle)
	{
		float newX = x * cos(angle) + z * sin(angle);
		float newZ = x * -sin(angle) + z * cos(angle);

		this->x = newX;
		this->z = newZ;
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