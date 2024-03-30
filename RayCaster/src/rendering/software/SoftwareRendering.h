#pragma once
#include "../RColor.h"
#include "../Camera.h"
#include "map/Raycast.h"
#include "../Rendering.h"

// #define USE_SOFTWARE_RENDERER

inline RColor* softwareRenderTarget;

constexpr size_t GetColorDataSize()
{
	return static_cast<size_t>(renderWidth) * renderHeight * sizeof(RColor);
}

void InitSoftwareRendering();
void CloseSoftwareRendering();

void RenderFrameSoftware(const Camera&, RColor* buffer, int width, int height);
Ray GetPixelRay(const int column, const int row, const Vector3& position,
	const Vector3& forwards, const Vector3& right, const Vector3& up);