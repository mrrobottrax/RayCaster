#include "pch.h"
#include "vk.h"

#include "vk_instance.h"
#include "vk_surface.h"
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_pipeline.h"
#include "vk_renderpass.h"
#include "vk_framebuffer.h"
#include "vk_command.h"

using namespace Vulkan;

void VK_Start()
{
	CreateInstance();
	CreateSurface();
	CreateDevice();
	CreateSwapChain();
	CreateRenderPass();
	CreateGraphicsPipeline();
	CreateFrameBuffers();
	CreateCommandPool();
	CreateCommandBuffer();
}

void VK_End()
{
	vkDestroyCommandPool(device, commandPool, nullptr);
	DestroyFrameBuffers();
	DestroyGraphicsPipeline();
	vkDestroyRenderPass(device, renderPass, nullptr);
	DestroySwapChain();
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyDevice(device, nullptr);
	vkDestroyInstance(instance, nullptr);
}