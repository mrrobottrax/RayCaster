#pragma once
#include <wtypes.h>

HWND InitWindow(WNDPROC, HINSTANCE);
void EndWindow(HWND, HINSTANCE);

ULONG_PTR InitGdi();
void CloseGdi(ULONG_PTR);

void KeyDown(WPARAM);
void KeyUp(WPARAM);

void InitConsole();
void CloseConsole();