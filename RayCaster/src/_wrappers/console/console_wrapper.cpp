#include "pch.h"
#include "console_wrapper.h"

#ifdef WINDOWS
#include <_platform/windows/console/w_console.h>
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

void Print(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	vprintf_s(format, args);

	va_end(args);
}
