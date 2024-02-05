#include "pch.h"
#include "Math.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution dis(-1.f, 1.f);

Vector3 RandomUnitVector()
{
	Vector3 vec(
		dis(gen),
		dis(gen),
		dis(gen)
	);

	vec.Normalize();

	return vec;
}

Vector3 RandomHemisphereVector(const Vector3& normal)
{
	Vector3 vec = RandomUnitVector();

	if (Vector3::Dot(vec, normal) < 0)
	{
		vec.Invert();
	}

	return vec;
}
