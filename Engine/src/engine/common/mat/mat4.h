#pragma once

#include "../vec/vec3.h"

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

	static mat4_base InverseTransformation(vec3_base<T> translation, T pitch, T yaw, T roll)
	{
		mat4_base<T> mat = mat4_base<T>::Identity();

		const T cP = cos(pitch);
		const T cY = cos(yaw);
		const T cR = cos(roll);

		const T sP = sin(pitch);
		const T sY = sin(yaw);
		const T sR = sin(roll);

		const T& x = translation.x;
		const T& y = translation.y;
		const T& z = translation.z;

		const T r0a = cR * cY;
		const T r0b = cP * sR + cR * sP * sY;
		const T r0c = sR * sP - cR * cP * sY;

		const T r1a = -cY * sR;
		const T r1b = cR * cP - sR * sP * sY;
		const T r1c = cP * sR * sY + cR * sP;

		const T r2a = sY;
		const T r2b = -cY * sP;
		const T r2c = cY * cP;

		mat.Set(0, 0, r0a);
		mat.Set(0, 1, r0b);
		mat.Set(0, 2, r0c);
		mat.Set(0, 3, -x * r0a - y * r0b - z * r0c);

		mat.Set(1, 0, r1a);
		mat.Set(1, 1, r1b);
		mat.Set(1, 2, r1c);
		mat.Set(1, 3, -x * r1a - y * r1b - z * r1c);

		mat.Set(2, 0, r2a);
		mat.Set(2, 1, r2b);
		mat.Set(2, 2, r2c);
		mat.Set(2, 3, -x * r2a - y * r2b - z * r2c);

		return mat;
	}
};

typedef mat4_base<float> mat4;