#pragma once
#include "color.h"

inline RColor* colorData;
constexpr int width = 640;
constexpr int height = 480;

inline size_t GetColorDataSize()
{
	return static_cast<size_t>(width) * height * sizeof(RColor);
}

void InitRendering();
void CloseRendering();

void RenderFrame();