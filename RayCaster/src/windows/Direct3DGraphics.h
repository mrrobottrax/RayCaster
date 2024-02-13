#pragma once
#include <wtypes.h>

void InitD3D11(HWND);
void EndD3D11();
void UpdateGpuMapD3D11();
void DrawFrameD3D11();
void CompileShaders();