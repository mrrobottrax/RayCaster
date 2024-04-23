#include "pch.h"
#include "vk_surface.h"

#include "windows/vk_w_window.h"

void CreateSurface()
{
#ifdef WINDOWS
	VK_W_CreateSurface();
#endif // WINDOWS
}