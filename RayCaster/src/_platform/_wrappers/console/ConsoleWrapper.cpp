#include "pch.h"
#include "ConsoleWrapper.h"

#ifdef WINDOWS
#include <_platform/windows/console/W_Console.h>
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
