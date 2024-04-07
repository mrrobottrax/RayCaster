#include "pch.h"
#include "graphics_wrapper.h"

#include <_platform/vulkan/vk_graphics.h>

void InitGraphics()
{
#ifdef VULKAN
	VK_Init();
#endif // VULKAN
}

void EndGraphics()
{
#ifdef VULKAN
	VK_End();
#endif // VULKAN
}