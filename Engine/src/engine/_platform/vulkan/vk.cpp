#include "pch.h"
#include "vk.h"

#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_pipeline.h"

void VK_Start()
{
	CreateInstance();
	CreateSurface();
	CreateDevice();
	CreateSwapChain();
	CreateGraphicsPipeline();
}

void VK_End()
{
	CleanUpSwapChain();
	vkDestroySurfaceKHR(Vulkan::instance, Vulkan::surface, nullptr);
	vkDestroyDevice(Vulkan::device, nullptr);
	vkDestroyInstance(Vulkan::instance, nullptr);
}