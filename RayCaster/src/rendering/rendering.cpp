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

constexpr int maxDepth = 2;
Vector3 TracePath(const Ray& ray, const int depth)
{
	if (depth >= maxDepth) {
		return Vector3();  // Bounced enough times.
	}

	const RaycastResult result = CastRay(ray);
	if (result.wallType == 0) {
		return result.normal / 2 + Vector3(0.5, 0.5, 0.5);  // Nothing was hit.
	}

	// Material material = ray.thingHit->material;
	// Color emittance = material.emittance;
	Vector3 emittance = result.normal;

	// Pick a random direction from here and keep going.
	Ray newRay;
	newRay.start = result.point;

	// This is NOT a cosine-weighted distribution!
	newRay.dir = RandomHemisphereVector(result.normal);

	// Probability of the newRay
	const float p = 1 / (2 * pi);

	// Compute the BRDF for this ray (assuming Lambertian reflection)
	float cos_theta = Vector3::Dot(newRay.dir, result.normal);
	// Color BRDF = material.reflectance / PI;
	float BRDF = 0.75 / pi;

	// Recursively trace reflected light sources.
	Vector3 incoming = TracePath(newRay, depth + 1);

	// Apply the Rendering Equation here.
	return emittance + (incoming * BRDF * cos_theta / p);
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