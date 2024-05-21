#pragma once

namespace Vulkan
{
	inline VkQueue graphicsQueue;
	inline VkQueue presentQueue;
}

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device);
bool QueueFamiliesAdequate(VkPhysicalDevice device);