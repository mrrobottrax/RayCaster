#include "pch.h"
#include "graphics_wrapper.h"

#ifdef VULKAN
#include "_platform/vulkan/vk.h"
#endif // VULKAN

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