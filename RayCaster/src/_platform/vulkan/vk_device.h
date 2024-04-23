#pragma once

void CreateDevice();

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool IsComplete() const
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);
bool IsDeviceSuitable(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);

std::vector<const char*> GetDeviceExtensionNames(VkPhysicalDevice device);

QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device);
SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);