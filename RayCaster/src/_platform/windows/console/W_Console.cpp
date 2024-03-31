#include "pch.h"
#include "W_Console.h"

void W_CreateConsole()
{
	AllocConsole();
	FILE* fDummy;
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
}

void W_DestroyConsole()
{
	// std::cin.get();
	FreeConsole();
}