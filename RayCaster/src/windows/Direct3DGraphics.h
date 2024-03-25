#pragma once
#include <wtypes.h>

bool InitD3D11(HWND);
void EndD3D11();
void UpdateMapTextureD3D11();
void DrawFrameD3D11();
void DrawSoftwareFrameD3D11();
void CompileShaders();