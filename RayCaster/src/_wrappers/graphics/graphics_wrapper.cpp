#include "pch.h"
#include "graphics_wrapper.h"

#include <_platform/vulkan/vk_graphics.h>

void StartGraphics()
{
#ifdef VULKAN
	VK_Start();
#endif // VULKAN
}

void EndGraphics()
{
#ifdef VULKAN
	VK_End();
#endif // VULKAN
}