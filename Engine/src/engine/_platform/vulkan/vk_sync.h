#pragma once

namespace Vulkan
{
	inline std::vector<VkSemaphore> imageAvailableSemaphores;
	inline std::vector<VkSemaphore> renderFinishedSemaphores;
	inline std::vector<VkFence> inFlightFences;
}

void CreateSyncObjects();
void DestroySyncObjects();