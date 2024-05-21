#include "pch.h"
#include "vk_sync.h"

#include "vk_device.h"

#include <_wrappers/graphics/graphics_constants.h>

using namespace Vulkan;

void CreateSyncObjects()
{
	imageAvailableSemaphore.resize(maxFramesInFlight);
	renderFinishedSemaphore.resize(maxFramesInFlight);
	inFlightFence.resize(maxFramesInFlight);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (int i = 0; i < maxFramesInFlight; ++i)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFence[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to creat sync objects");
		}
	}
}

void DestroySyncObjects()
{
	for (int i = 0; i < maxFramesInFlight; ++i)
	{
		vkDestroySemaphore(device, imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(device, renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(device, inFlightFence[i], nullptr);
	}
}