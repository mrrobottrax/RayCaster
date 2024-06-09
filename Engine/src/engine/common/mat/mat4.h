#pragma once

template <typename T>
class mat4_base
{
public:
	T data[16];

	void Set(unsigned int row, unsigned int column, T value)
	{
		data[column * 4 + row] = value;
	}

	static mat4_base Identity()
	{
		mat4_base matrix{};

		matrix.Set(0, 0, 1);
		matrix.Set(1, 1, 1);
		matrix.Set(2, 2, 1);
		matrix.Set(3, 3, 1);

		return  matrix;
	}
};

typedef mat4_base<float> mat4;