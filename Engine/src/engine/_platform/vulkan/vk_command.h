#pragma once

namespace Vulkan
{
	inline VkCommandPool commandPool;
	inline std::vector<VkCommandBuffer> commandBuffer;
}

void CreateCommandPool();
void CreateCommandBuffers();
void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);