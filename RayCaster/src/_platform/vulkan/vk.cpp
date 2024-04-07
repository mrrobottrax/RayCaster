#include "pch.h"
#include "vk.h"

#include "vk_graphics.h"

void VK_Start()
{
	CreateInstance();
	CreateDevice();

	// vkCreateWin32SurfaceKHR();
}

void VK_End()
{
	vkDestroyDevice(VK::device, nullptr);
	vkDestroyInstance(VK::instance, nullptr);
}