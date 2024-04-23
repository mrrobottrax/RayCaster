#pragma once

#pragma warning(push)
#pragma warning(disable : 26495)
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
#pragma warning(pop)

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);