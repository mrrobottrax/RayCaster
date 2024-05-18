#pragma once

#include "windows.h"
#include "wtypes.h"
#include "fileapi.h"

API void W_EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
)
{
	W_EntryPoint(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}