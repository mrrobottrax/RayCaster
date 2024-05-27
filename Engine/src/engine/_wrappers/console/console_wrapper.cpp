#include "pch.h"
#include "console_wrapper.h"

#ifdef WINDOWS
#include "_platform/windows/console/w_console.h"
#endif // WINDOWS

void CreateConsole()
{
#ifdef WINDOWS
	W_CreateConsole();
#endif
}

void DestroyConsole()
{
#ifdef WINDOWS
	W_DestroyConsole();
#endif
}