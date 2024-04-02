#include "pch.h"
#include "VK_Graphics.h"

void VK_Init()
{
	/*VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Raytrace Game";
	appInfo.applicationVersion = 0;
	appInfo.pEngineName = "MCP Engine";
	appInfo.engineVersion = 0;
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
	createInfo.enabledExtensionCount = 0;
	createInfo.ppEnabledExtensionNames = nullptr;

	VkResult result = vkCreateInstance(&createInfo, nullptr, pVkInstance);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create instance!");*/

	// vkCreateWin32SurfaceKHR();
}
