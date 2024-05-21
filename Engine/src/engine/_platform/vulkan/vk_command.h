#pragma once

namespace Vulkan
{
	inline VkCommandPool commandPool;
	inline VkCommandBuffer commandBuffer;
}

void CreateCommandPool();
void CreateCommandBuffer();
void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);