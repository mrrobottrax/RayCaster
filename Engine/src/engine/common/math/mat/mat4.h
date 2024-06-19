#pragma once

template <typename T>
class vec3_base;

template <typename T>
class vec4_base;

template <typename T>
class mat4_base
{
public:
	T data[16];

	mat4_base() : data{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}
	{}

	constexpr void set(unsigned int row, unsigned int column, T value)
	{
		data[column * 4 + row] = value;
	}

	constexpr T get(unsigned int row, unsigned int column) const
	{
		return data[column * 4 + row];
	}

	constexpr static mat4_base identity()
	{
		mat4_base matrix{};

		matrix.set(0, 0, 1);
		matrix.set(1, 1, 1);
		matrix.set(2, 2, 1);
		matrix.set(3, 3, 1);

		return matrix;
	}

	constexpr mat4_base times(const mat4_base& rhs) const
	{
		vec4_base<T> column0(rhs.get(0, 0), rhs.get(1, 0), rhs.get(2, 0), rhs.get(3, 0));
		vec4_base<T> column1(rhs.get(0, 1), rhs.get(1, 1), rhs.get(2, 1), rhs.get(3, 1));
		vec4_base<T> column2(rhs.get(0, 2), rhs.get(1, 2), rhs.get(2, 2), rhs.get(3, 2));
		vec4_base<T> column3(rhs.get(0, 3), rhs.get(1, 3), rhs.get(2, 3), rhs.get(3, 3));

		vec4_base<T> row0(get(0, 0), get(0, 1), get(0, 2), get(0, 3));
		vec4_base<T> row1(get(1, 0), get(1, 1), get(1, 2), get(1, 3));
		vec4_base<T> row2(get(2, 0), get(2, 1), get(2, 2), get(2, 3));
		vec4_base<T> row3(get(3, 0), get(3, 1), get(3, 2), get(3, 3));

		mat4_base out;

		out.set(0, 0, row0.dot(column0));
		out.set(1, 0, row1.dot(column0));
		out.set(2, 0, row2.dot(column0));
		out.set(3, 0, row3.dot(column0));

		out.set(0, 1, row0.dot(column1));
		out.set(1, 1, row1.dot(column1));
		out.set(2, 1, row2.dot(column1));
		out.set(3, 1, row3.dot(column1));

		out.set(0, 2, row0.dot(column2));
		out.set(1, 2, row1.dot(column2));
		out.set(2, 2, row2.dot(column2));
		out.set(3, 2, row3.dot(column2));

		out.set(0, 3, row0.dot(column3));
		out.set(1, 3, row1.dot(column3));
		out.set(2, 3, row2.dot(column3));
		out.set(3, 3, row3.dot(column3));

		return out;
	}

	// Generate a transformation matrix
	template <typename U>
	constexpr static mat4_base transformation(vec3_base<U> translation, vec3_base<U> angles)
	{
		mat4_base translationMatrix{};
		translationMatrix.set(0, 3, translation.x);
		translationMatrix.set(1, 3, translation.y);
		translationMatrix.set(2, 3, translation.z);

		const U pitch = angles.x;
		const U yaw = angles.y;
		const U roll = angles.z;

		const U cP = cos(pitch);
		const U cY = cos(yaw);
		const U cR = cos(roll);

		const U sP = sin(pitch);
		const U sY = sin(yaw);
		const U sR = sin(roll);

		mat4_base pitchMatrix{};
		pitchMatrix.set(1, 1, cP);
		pitchMatrix.set(1, 2, -sP);
		pitchMatrix.set(2, 1, sP);
		pitchMatrix.set(2, 2, cP);

		mat4_base yawMatrix{};
		yawMatrix.set(0, 0, cY);
		yawMatrix.set(0, 2, -sY);
		yawMatrix.set(2, 0, sY);
		yawMatrix.set(2, 2, cY);

		mat4_base rollMatrix{};
		rollMatrix.set(0, 0, cR);
		rollMatrix.set(0, 1, -sR);
		rollMatrix.set(1, 0, sR);
		rollMatrix.set(1, 1, cR);

		return translationMatrix.times(yawMatrix.times(pitchMatrix.times(rollMatrix)));
	}

	// Generate an inverse transformation matrix (view matrix)
	template <typename U>
	constexpr static mat4_base view(vec3_base<U> translation, vec3_base<U> angles)
	{
		mat4_base translationMatrix{};
		translationMatrix.set(0, 3, -translation.x);
		translationMatrix.set(1, 3, -translation.y);
		translationMatrix.set(2, 3, -translation.z);

		const U pitch = angles.x;
		const U yaw = -angles.y;
		const U roll = -angles.z;

		const U cP = cos(pitch);
		const U cY = cos(yaw);
		const U cR = cos(roll);

		const U sP = sin(pitch);
		const U sY = sin(yaw);
		const U sR = sin(roll);

		mat4_base pitchMatrix{};
		pitchMatrix.set(1, 1, cP);
		pitchMatrix.set(1, 2, -sP);
		pitchMatrix.set(2, 1, sP);
		pitchMatrix.set(2, 2, cP);

		mat4_base yawMatrix{};
		yawMatrix.set(0, 0, cY);
		yawMatrix.set(0, 2, -sY);
		yawMatrix.set(2, 0, sY);
		yawMatrix.set(2, 2, cY);

		mat4_base rollMatrix{};
		rollMatrix.set(0, 0, cR);
		rollMatrix.set(0, 1, -sR);
		rollMatrix.set(1, 0, sR);
		rollMatrix.set(1, 1, cR);

		return rollMatrix.times(pitchMatrix.times(yawMatrix.times(translationMatrix)));
	}

	// Generate a perspective matrix
	constexpr static mat4_base perspective(float aspect)
	{
		mat4_base mat{};

		mat.set(0, 0, 1.0f / aspect);
		mat.set(1, 1, -1);
		mat.set(3, 3, 0);
		mat.set(3, 2, 1);

		return mat;
	}
};

typedef mat4_base<float> mat4;