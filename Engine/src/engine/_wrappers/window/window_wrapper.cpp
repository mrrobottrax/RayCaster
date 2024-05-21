#include "pch.h"
#include "window_wrapper.h"

#ifdef WINDOWS
#include "_platform/windows/window/w_window.h"
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

void GetMainWindowSize(uint32_t* pWidth, uint32_t* pHeight)
{
#ifdef WINDOWS
	W_GetMainWindowClientSize(pWidth, pHeight);
#endif // WINDOWS
}

void WaitEvents()
{
#ifdef WINDOWS
	W_WaitEvents();
#endif // WINDOWS
}
