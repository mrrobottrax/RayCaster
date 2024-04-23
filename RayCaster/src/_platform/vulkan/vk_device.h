#pragma once

namespace Vk
{
	inline VkPhysicalDevice physicalDevice;
	inline VkDevice device;
}

void CreateDevice();

VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);
bool IsDeviceSuitable(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);

bool GetDeviceExtensionSupport(VkPhysicalDevice device);