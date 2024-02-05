#pragma once

template <typename T>
class Vector2Base
{
public:
	T x;
	T y;

public:
	constexpr Vector2Base() : x(0), y(0)
	{}

	constexpr Vector2Base(T x, T y) : x(x), y(y)
	{}

public:
	static constexpr float Dot(const Vector2Base& a, const Vector2Base& b)
	{
		return static_cast<float>(a.x * b.x + a.y * b.y);
	}

	static constexpr float Dist(const Vector2Base& a, const Vector2Base& b)
	{
		const float i = a.x - b.x;
		const float j = a.y - b.y;

		return sqrtf(i * i + j * j);
	}

	constexpr void Rotate(float angle)
	{
		const float x = this->x;

		this->x = this->x * cos(angle) + this->y * -sin(angle);
		this->y = x * sin(angle) + this->y * cos(angle);
	}
};

template <typename T>
class Vector3Base
{
public:
	T x;
	T y;
	T z;

public:
	constexpr Vector3Base() : x(0), y(0), z(0)
	{}

	constexpr Vector3Base(T x, T y, T z) : x(x), y(y), z(z)
	{}

public:
	constexpr void Invert()
	{
		this->x *= -1;
		this->y *= -1;
		this->z *= -1;
	}

	constexpr float Magnitude() const
	{
		return sqrt(static_cast<float>(x * x + y * y + z * z));
	}

	static constexpr float Dot(const Vector3Base& a, const Vector3Base& b)
	{
		return static_cast<float>(a.x * b.x + a.y * b.y + a.z * b.z);
	}
};

class Vector2 : public Vector2Base<float>
{
public:
	constexpr Vector2() : Vector2Base()
	{}

	constexpr Vector2(float x, float y) : Vector2Base(x, y)
	{}

public:
	constexpr Vector2 operator+(const Vector2Base& rhs) const
	{
		return Vector2(
			this->x + rhs.x,
			this->y + rhs.y
		);
	}

	constexpr Vector2& operator+=(const Vector2Base& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;

		return *this;
	}

	constexpr Vector2& operator*=(const float rhs)
	{
		this->x *= rhs;
		this->y *= rhs;

		return *this;
	}
};

class Vector2Int : public Vector2Base<int>
{
public:
	constexpr Vector2Int() : Vector2Base()
	{}

	constexpr Vector2Int(int x, int y) : Vector2Base(x, y)
	{}

public:
	constexpr Vector2Int operator+(const Vector2Base& rhs) const
	{
		return Vector2Int(
			this->x + rhs.x,
			this->y + rhs.y
		);
	}
};

class Vector3 : public Vector3Base<float>
{
public:
	constexpr Vector3() : Vector3Base()
	{}

	constexpr Vector3(float x, float y, float z) : Vector3Base(x, y, z)
	{}

public:
	constexpr Vector3 operator+(const Vector3Base& rhs) const
	{
		return Vector3(
			this->x + rhs.x,
			this->y + rhs.y,
			this->z + rhs.z
		);
	}

	constexpr Vector3& operator+=(const Vector3Base& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;

		return *this;
	}

	constexpr Vector3 operator*(const float rhs) const
	{
		return Vector3(
			this->x * rhs,
			this->y * rhs,
			this->z * rhs
		);
	}

	constexpr Vector3 operator/(const float rhs) const
	{
		return Vector3(
			this->x / rhs,
			this->y / rhs,
			this->z / rhs
		);
	}

	constexpr Vector3& operator/=(const float rhs)
	{
		this->x /= rhs;
		this->y /= rhs;
		this->z /= rhs;

		return *this;
	}

public:
	constexpr void Normalize()
	{
		*this /= Magnitude();
	}
};