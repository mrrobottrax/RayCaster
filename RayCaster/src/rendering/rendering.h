#pragma once

inline BYTE* colorData;
constexpr int width = 640;
constexpr int height = 480;
constexpr int bytesPerPixel = 2;

void InitRendering();
void CloseRendering();

void RenderFrame();