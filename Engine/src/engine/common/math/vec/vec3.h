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

	// ~~~~~~~~~~~~~~~~~ Constructors ~~~~~~~~~~~~~

	vec3_base() : x(0), y(0), z(0)
	{}

	vec3_base(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
	{}

	// ~~~~~~~~~~~~~~~~~ OPERATORS ~~~~~~~~~~~~

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

	// Conversion
	template <typename U>
	constexpr operator vec3_base<U>() const
	{
		return vec3_base<U>(static_cast<U>(this->x), static_cast<U>(this->y), static_cast<U>(this->z));
	}

	// ~~~~~~~~~~~~~~~~~~ METHODS ~~~~~~~~~~~~~~~~

	template <typename U>
	constexpr vec3_base rotate(vec3_base<U> angles)
	{
		return rotate(angles.x, angles.y, angles.z);
	}

	template <typename U>
	constexpr vec3_base rotate(U pitch = 0, U yaw = 0, U roll = 0)
	{
		vec3_base v{};

		const T cP = cos(pitch);
		const T cY = cos(yaw);
		const T cR = cos(roll);

		const T sP = sin(pitch);
		const T sY = sin(yaw);
		const T sR = sin(roll);

		v.x = this->x * cR * cY + this->y * (cP * sR + sP * cR * sY) + this->z * (sP * sR - cP * cR * sY);
		v.y = -this->x * sR * cY + this->y * (cP * cR - sP * sR * sY) + this->z * (sP * cR - cP * sR * sY);
		v.z = this->x * sY - this->y * sP * cY + this->z * cP * cY;

		return v;
	}

	constexpr vec3_base normalize()
	{
		T sqrMagnitude = this->x * this->x + this->y * this->y + this->z * this->z;

		if (sqrMagnitude == 0)
		{
			return vec3_base(*this);
		}

		T magnitude = sqrt(sqrMagnitude);

		vec3_base v(*this);
		v.x /= magnitude;
		v.y /= magnitude;
		v.z /= magnitude;
		return v;
	}
};

typedef vec3_base<float> vec3;
typedef vec3_base<uint32_t> ivec3;