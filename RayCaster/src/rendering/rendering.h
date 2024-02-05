#pragma once
#include "RColor.h"
#include "Camera.h"
#include "map/Raycast.h"

inline RColor* viewColorBuffer;
constexpr int viewWidth = 320;
constexpr int viewHeight = 200;

constexpr int renderScale = 1;

inline size_t GetColorDataSize()
{
	return static_cast<size_t>(viewWidth) * viewHeight * sizeof(RColor);
}

void InitRendering();
void CloseRendering();

void RenderFrame(Camera&);
Ray GetPixelRay(const int column, const int row, const Camera&,
	const Vector3& forwards, const Vector3& right, const Vector3& up);