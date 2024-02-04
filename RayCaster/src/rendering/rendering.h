#pragma once
#include "RColor.h"
#include <player/Camera.h>
#include "ScanLine.h"

inline RColor* viewColorBuffer;
constexpr int width = 400;
constexpr int height = 300;

inline size_t GetColorDataSize()
{
	return static_cast<size_t>(width) * height * sizeof(RColor);
}

void InitRendering();
void CloseRendering();

void RenderFrame(Camera&);
ScanLine GetScanLine(const Vector2& position, const float angle, const Vector2& forwards);