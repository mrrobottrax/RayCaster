#pragma once

template <typename T>
class vec4_base
{
public:
	union
	{
		struct { T x; T y; T z; T w; };
		struct { T r; T g; T b; T a; };
	};

	// Constructors

	vec4_base() : x(0), y(0), z(0), w(0)
	{}

	vec4_base(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w)
	{}

	// ~~~~~~~~~~~~~~~~~~~ OPERATORS ~~~~~~~~~~~~

	// Addition
	constexpr vec4_base<T>& operator+ (vec4_base<T>&& v) const
	{
		v.x += this->x;
		v.y += this->y;
		v.z += this->z;
		v.w += this->w;
		return v;
	}

	constexpr vec4_base<T> operator+ (const vec4_base<T>& v) const
	{
		return std::move(vec4_base<T>{this->x + v.x, this->y + v.y, this->z + v.z, this->w + v.w});
	}

	constexpr vec4_base<T>& operator+= (const vec4_base<T>& v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		this->w += v.w;
		return *this;
	}

	// Multiplication
	constexpr vec4_base<T> operator* (const float f) const
	{
		return vec4_base<T>{this->x * f, this->y * f, this->z * f, this->w * f};
	}

	constexpr vec4_base<T>& operator*= (const float f)
	{
		this->x *= f;
		this->y *= f;
		this->z *= f;
		this->w *= f;
		return *this;
	}

	// Conversion

	template <typename U>
	constexpr operator vec4_base<U>()
	{
		return vec4_base<U>(static_cast<U>(this->x), static_cast<U>(this->y), static_cast<U>(this->z), static_cast<U>(this->w));
	}

	// ~~~~~~~~~~~~~~~~~~ METHODS ~~~~~~~~~~~~~

	constexpr vec4_base normalize()
	{
		T sqrMagnitude = this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;

		if (sqrMagnitude == 0)
		{
			return vec4_base(*this);
		}

		T magnitude = sqrt(sqrMagnitude);

		vec4_base v(*this);
		v.x /= magnitude;
		v.y /= magnitude;
		v.z /= magnitude;
		v.w /= magnitude;
		return v;
	}

	constexpr T dot(vec4_base vec) const
	{
		return x * vec.x + y * vec.y + z * vec.z + w * vec.w;
	}
};

typedef vec4_base<float> vec4;
typedef vec4_base<uint32_t> ivec4;