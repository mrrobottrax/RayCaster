#pragma once

void CreateInstance();
void CreateDevice();
void CreateSurface();

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const {
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

std::vector<const char*> GetInstanceLayerNames();
std::vector<const char*> GetInstanceExtensionNames();
VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
bool IsDeviceSuitable(VkPhysicalDevice, const VkPhysicalDeviceProperties&);
QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice);
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice);
std::vector<const char*> GetDeviceExtensionNames(VkPhysicalDevice);