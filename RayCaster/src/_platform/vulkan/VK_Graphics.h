#pragma once

namespace VK
{
	inline VkInstance instance;
	inline VkPhysicalDevice physicalDevice;
	inline VkDevice device;

	inline VkQueue graphicsQueue;
}

void CreateInstance();
void CreateDevice();

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value();
	}
};

std::vector<const char*> GetEnabledInstanceLayerNames();
VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
bool IsDeviceSuitable(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);