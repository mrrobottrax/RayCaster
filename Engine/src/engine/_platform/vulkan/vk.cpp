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
	vkDestroySurfaceKHR(Vk::instance, Vk::surface, nullptr);
	vkDestroyDevice(Vk::device, nullptr);
	vkDestroyInstance(Vk::instance, nullptr);
}