#include "pch.h"
#include "vk.h"

#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"

void VK_Start()
{
	CreateInstance();
	CreateSurface();
	CreateDevice();
}

void VK_End()
{
	vkDestroySurfaceKHR(VK::instance, VK::surface, nullptr);
	vkDestroyDevice(VK::device, nullptr);
	vkDestroyInstance(VK::instance, nullptr);
}