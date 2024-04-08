#pragma once

namespace VK
{
	inline VkInstance instance;
	inline VkPhysicalDevice physicalDevice;
	inline VkDevice device;

	inline VkQueue graphicsQueue;
	inline VkSurfaceKHR surface;
}

void CreateInstance();
void CreateDevice();
void CreateSurface();
void DestroySurface();

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value();
	}
};

std::vector<const char*> GetInstanceLayerNames();
std::vector<const char*> GetInstanceExtensionNames();
VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
bool IsDeviceSuitable(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
std::vector<const char*> GetDeviceExtensionNames(VkPhysicalDevice);