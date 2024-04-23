#include "pch.h"
#include "vk.h"

#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"

void VK_Start()
{
	CreateInstance();
	CreateSurface();
	CreateDevice();
	CreateSwapChain();
}

void VK_End()
{
	vkDestroySwapchainKHR(Vk::device, Vk::swapChain, nullptr);
	vkDestroySurfaceKHR(Vk::instance, Vk::surface, nullptr);
	vkDestroyDevice(Vk::device, nullptr);
	vkDestroyInstance(Vk::instance, nullptr);
}