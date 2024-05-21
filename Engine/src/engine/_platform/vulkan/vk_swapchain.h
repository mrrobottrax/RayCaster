#pragma once

namespace Vulkan
{
	inline VkSwapchainKHR swapChain;
	inline std::vector<VkImage> swapChainImages;
	inline std::vector<VkImageView> swapChainImageViews;
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
void DestroySwapChain();

bool SwapChainAdequate(VkPhysicalDevice device);