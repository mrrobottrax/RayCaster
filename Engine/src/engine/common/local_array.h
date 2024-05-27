#pragma once

template<typename T>
class LocalArray
{
public:
	T* data;

	constexpr LocalArray(uint32_t size)
	{
		data = new T[size];
	}

	constexpr ~LocalArray()
	{
		delete data;
	}

	constexpr T& operator[](uint32_t index)
	{
		return data[index];
	}
};