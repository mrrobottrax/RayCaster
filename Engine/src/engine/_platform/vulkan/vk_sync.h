#pragma once

namespace Vulkan
{
	inline std::vector<VkSemaphore> imageAvailableSemaphore;
	inline std::vector<VkSemaphore> renderFinishedSemaphore;
	inline std::vector<VkFence> inFlightFence;
}

void CreateSyncObjects();
void DestroySyncObjects();