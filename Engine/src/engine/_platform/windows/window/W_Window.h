#pragma once

namespace W_Window
{
	inline HWND hWnd;
}

void W_CreateMainWindow();
void W_DestroyMainWindow();

void W_GetMainWindowClientSize(uint32_t* pWidth, uint32_t* pHeight);