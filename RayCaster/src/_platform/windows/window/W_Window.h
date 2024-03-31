#pragma once

LRESULT CALLBACK W_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

namespace W_MainWindow
{
	inline HWND hWnd;
	inline int nCmdShow;
}

void W_CreateMainWindow();
void W_DestroyMainWindow();