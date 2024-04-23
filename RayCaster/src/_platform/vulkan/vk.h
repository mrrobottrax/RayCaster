#pragma once

void VK_Start();
void VK_End();

namespace VK
{
	inline VkInstance instance;
	inline VkPhysicalDevice physicalDevice;
	inline VkDevice device;

	inline VkSurfaceKHR surface;

	namespace Queue
	{
		inline VkQueue graphicsQueue;
		inline VkQueue presentQueue;
	}
}