#pragma once
#include "RColor.h"
#include "ScanLine.h"
#include "Camera.h"

inline RColor* viewColorBuffer;
constexpr int viewWidth = 400;
constexpr int viewHeight = 300;

constexpr int renderScale = 1;

inline size_t GetColorDataSize()
{
	return static_cast<size_t>(viewWidth) * viewHeight * sizeof(RColor);
}

void InitRendering();
void CloseRendering();

void RenderFrame(Camera&);
ScanLine GetScanLine(const Vector2& position, const float angle, const Vector2& forwards);