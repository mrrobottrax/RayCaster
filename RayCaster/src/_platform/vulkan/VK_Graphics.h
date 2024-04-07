#pragma once

void VK_Start();
void VK_End();

// Internal
namespace VK
{
	inline VkInstance instance;
	inline VkPhysicalDevice physicalDevice;

	void CreateInstance();
	std::vector<const char*> GetEnabledInstanceLayerNames();

	void CreateDevice();
	VkPhysicalDevice PickPhysicalDevice();
	int RatePhysicalDeviceSuitability(const VkPhysicalDeviceProperties&);
}