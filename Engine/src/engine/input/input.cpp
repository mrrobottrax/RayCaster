#include "pch.h"
#include "input.h"

#ifdef WINDOWS
#include <_platform/windows/input/w_input.h>
#endif // WINDOWS


void InitInput()
{
#ifdef WINDOWS
	W_InitInput();
#endif // WINDOWS
}