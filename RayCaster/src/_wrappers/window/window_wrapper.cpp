#include "pch.h"
#include "window_wrapper.h"

#ifdef WINDOWS
#include <_platform/windows/window/w_window.h>
#endif // WINDOWS


void CreateMainWindow()
{
#ifdef WINDOWS
	W_CreateMainWindow();
#endif
}

void DestroyMainWindow()
{
#ifdef WINDOWS
	W_DestroyMainWindow();
#endif
}
