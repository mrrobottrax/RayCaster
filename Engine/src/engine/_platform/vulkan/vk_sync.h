#pragma once

namespace Vulkan
{
	inline VkSemaphore imageAvailableSemaphore;
	inline VkSemaphore renderFinishedSemaphore;
	inline VkFence inFlightFence;
}

void CreateSyncObjects();
void DestroySyncObjects();