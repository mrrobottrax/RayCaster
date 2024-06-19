#include "pch.h"
#include "input.h"
#include "mouse.h"
#include "button.h"

#ifdef WINDOWS
#include <_platform/windows/input/w_input.h>
#endif // WINDOWS


void UpdateInput()
{
	ResetMouseDelta();

#ifdef WINDOWS
	W_RawInputBuffer();
#endif // WINDOWS
}