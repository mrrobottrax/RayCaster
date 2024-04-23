#pragma once

namespace Vk
{
	inline VkSwapchainKHR swapChain;
	inline std::vector<VkImage> swapChainImages;
	inline VkFormat swapChainImageFormat;
	inline VkExtent2D swapChainExtent;
}

#pragma warning(push)
#pragma warning(disable : 26495)
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};
#pragma warning(pop)

void CreateSwapChain();

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availableModes);
VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

inline bool SwapChainAdequate(VkPhysicalDevice device)
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
	return !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
}