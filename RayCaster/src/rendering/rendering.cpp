#include <pch.h>
#include "Rendering.h"
#include <game/Game.h>
#include <map/Map.h>
#include <common/Math.h>

void InitRendering()
{
	viewColorBuffer = new RColor[viewWidth * viewHeight];
	memset(viewColorBuffer, 0, GetColorDataSize());
}

void CloseRendering()
{
	delete[] viewColorBuffer;
}

void RenderFrame(Camera& camera)
{
	camera.RenderFrame(viewColorBuffer);
}

constexpr int maxDepth = 3;
Vector3 TracePath(const Ray& ray, const int depth)
{
	if (depth >= maxDepth) {
		return Vector3();  // Bounced enough times.
	}

	const RaycastResult result = CastRay(ray);
	if (result.wallType == 0) {
		return Vector3(1, 1, 1);  // Nothing was hit.
	}

	// Material material = ray.thingHit->material;
	// Color emittance = material.emittance;
	Vector3 emittance = Vector3();

	// Pick a random direction from here and keep going.
	Ray newRay;
	newRay.start = result.point + result.normal * 0.01f;

	// This is NOT a cosine-weighted distribution!
	newRay.dir = RandomHemisphereVector(result.normal);

	// Probability of the newRay
	constexpr float p = 1 / (2.f * pi);

	// Compute the BRDF for this ray (assuming Lambertian reflection)
	float cos_theta = Vector3::Dot(newRay.dir, result.normal);
	// Color BRDF = material.reflectance / PI;
	Vector3 BRDF = Vector3(
		abs(result.normal.x),
		abs(result.normal.y),
		abs(result.normal.z)
	) / pi;

	// Recursively trace reflected light sources.
	Vector3 incoming = TracePath(newRay, depth + 1);

	Vector3 mult = Vector3(
		incoming.x * BRDF.x,
		incoming.y * BRDF.y,
		incoming.z * BRDF.z
	);

	// Apply the Rendering Equation here.
	return emittance + (mult * cos_theta / p);
}

Ray GetPixelRay(const int column, const int row, const Camera& camera,
	const Vector3& forwards, const Vector3& right, const Vector3& up)
{
	float rightScale = column / static_cast<float>(viewWidth) * 2.f - 1;
	float upScale = -row / static_cast<float>(viewHeight) * 2.f + 1;

	Vector3 dir = forwards + right * rightScale + up * upScale;
	dir.Normalize();

	return {
		camera.position,
		dir
	};
}