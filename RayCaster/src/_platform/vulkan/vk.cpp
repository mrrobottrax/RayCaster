#include "pch.h"
#include "vk.h"

#include "vk_graphics.h"

void VK_Start()
{
	CreateInstance();
	CreateDevice();
	CreateSurface();
}

void VK_End()
{
	DestroySurface();
	vkDestroyDevice(VK::device, nullptr);
	vkDestroyInstance(VK::instance, nullptr);
}