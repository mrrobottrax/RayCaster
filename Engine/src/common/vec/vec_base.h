#pragma once

// All vectors share these functions
template <size_t size, typename T>
struct vec_base
{
public:
	virtual T SqrMagnitude() = 0;
	virtual void Invert() = 0;

public:
	float Magnitude()
	{
		return sqrtf(static_cast<float>(SqrMagnitude()));
	}
};

// Float vectors get extra functions
struct vec_base_float
{
public:
	virtual void Normalize() = 0;
};

// Template
template <size_t size, typename T>
struct vec : vec_base<size, T>
{};