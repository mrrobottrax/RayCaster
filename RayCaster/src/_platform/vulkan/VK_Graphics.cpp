#include "pch.h"
#include "vk_graphics.h"

#include <_wrappers/console/console_wrapper.h>

// Get a vector of all enabled layer names
static std::vector<const char*> GetEnabledInstanceLayerNames()
{
	uint32_t availableLayersCount = 0;
	vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);

	std::vector<VkLayerProperties> availableLayers = std::vector<VkLayerProperties>(availableLayersCount);
	vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

#ifdef _DEBUG
	Println("Layers:\n");
	for (VkLayerProperties layer : availableLayers)
	{
		Println("%s", layer.layerName);
	}
	Println("");
#endif // _DEBUG

	std::vector<const char*> requiredLayerNames = std::vector<const char*>();

#ifndef NDEBUG
	requiredLayerNames.push_back("VK_LAYER_KHRONOS_validation");
	requiredLayerNames.push_back("VK_LAYER_KHRONOS_synchronization2");
#endif // !NDEBUG

	for (const char* layerName : requiredLayerNames)
	{
		// check if this layer is available
		bool layerAvailable = false;
		for (const VkLayerProperties& layer : availableLayers)
		{
			if (!strcmp(layerName, layer.layerName))
			{
				layerAvailable = true;
				break;
			}
		}

		if (!layerAvailable)
		{
			throw std::runtime_error(std::format("Validation layer {} required but not available", layerName));
		}
	}

	return requiredLayerNames;
}

void VK_Init()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Raytrace Game";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "MCP Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	std::vector<const char*> enabledInstanceLayerNames = GetEnabledInstanceLayerNames();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(enabledInstanceLayerNames.size());
	createInfo.ppEnabledLayerNames = enabledInstanceLayerNames.data();
	createInfo.enabledExtensionCount = 0;
	createInfo.ppEnabledExtensionNames = nullptr;

	if (vkCreateInstance(&createInfo, nullptr, &vkInstance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}

	// vkCreateWin32SurfaceKHR();
}

void VK_End()
{
	vkDestroyInstance(vkInstance, nullptr);
}