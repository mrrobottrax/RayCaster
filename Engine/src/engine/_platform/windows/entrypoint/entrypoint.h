#pragma once

#include "Windows.h"

#include "engine/setup/setup.h"

API void W_EntryPoint(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow);

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow
)
{
	Setup();
	W_EntryPoint(hInstance, hPrevInstance, pCmdLine, nCmdShow);
}