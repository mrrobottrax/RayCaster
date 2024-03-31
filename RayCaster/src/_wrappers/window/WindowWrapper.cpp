#include "pch.h"
#include "WindowWrapper.h"

#ifdef WINDOWS
#include <_platform/windows/window/W_Window.h>
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
