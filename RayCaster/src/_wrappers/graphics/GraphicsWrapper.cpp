#include "pch.h"
#include "GraphicsWrapper.h"

#include <_platform/vulkan/VK_Graphics.h>

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