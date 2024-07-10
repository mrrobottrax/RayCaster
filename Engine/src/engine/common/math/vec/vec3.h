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

	constexpr vec3_base(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
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

	// Subtraction
	constexpr vec3_base<T>& operator- (vec3_base<T>&& v) const
	{
		v.x -= this->x;
		v.y -= this->y;
		v.z -= this->z;
		return v;
	}

	constexpr vec3_base<T> operator- (const vec3_base<T>& v) const
	{
		return std::move(vec3_base<T>{this->x - v.x, this->y - v.y, this->z - v.z});
	}

	constexpr vec3_base<T>& operator-= (const vec3_base<T>& v)
	{
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
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

	// Division
	constexpr vec3_base<T> operator/ (const float f) const
	{
		return vec3_base<T>{this->x / f, this->y / f, this->z / f};
	}

	constexpr vec3_base<T>& operator/= (const float f)
	{
		this->x /= f;
		this->y /= f;
		this->z /= f;
		return *this;
	}

	// Conversion
	template <typename U>
	constexpr operator vec3_base<U>() const
	{
		return vec3_base<U>(static_cast<U>(this->x), static_cast<U>(this->y), static_cast<U>(this->z));
	}

	// Access
	constexpr const T& operator[](int i) const
	{
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return x;
		}
	}

	constexpr T& operator[](int i)
	{
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		default:
			return x;
		}
	}

	// ~~~~~~~~~~~~~~~~~~ METHODS ~~~~~~~~~~~~~~~~

	constexpr static vec3_base min(const vec3_base& a, const vec3_base& b)
	{
		vec3_base out;
		out.x = std::min(a.x, b.x);
		out.y = std::min(a.y, b.y);
		out.z = std::min(a.z, b.z);

		return out;
	}

	constexpr static vec3_base max(const vec3_base& a, const vec3_base& b)
	{
		vec3_base out;
		out.x = std::max(a.x, b.x);
		out.y = std::max(a.y, b.y);
		out.z = std::max(a.z, b.z);

		return out;
	}

	constexpr static float dot(const vec3_base& a, const vec3_base& b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	constexpr static vec3_base lerp(const vec3_base& a, const vec3_base& b, float t)
	{
		vec3_base out;

		for (int i = 0; i < 3; ++i)
		{
			out[i] = a[i] + t * (b[i] - a[i]);
		}

		return out;
	}

	constexpr vec3_base rotate(const vec3_base& angles)
	{
		return rotate(angles.x, angles.y, angles.z);
	}

	constexpr vec3_base rotate(T pitch = 0, T yaw = 0, T roll = 0)
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

	constexpr T magnitude()
	{
		T sqrMagnitude = this->x * this->x + this->y * this->y + this->z * this->z;

		if (sqrMagnitude == 0)
		{
			return 0;
		}

		T magnitude = sqrt(sqrMagnitude);
		return magnitude;
	}
};

typedef vec3_base<float> vec3;
typedef vec3_base<uint32_t> uvec3;