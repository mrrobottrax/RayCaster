#pragma once
#include <wtypes.h>

HWND InitWindow(WNDPROC, HINSTANCE);
ULONG_PTR InitGdi();
void CloseGdi(ULONG_PTR);