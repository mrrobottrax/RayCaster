#include "core_pch.h"
#include "vk_instance.h"

#include "vk.h"
#include <_wrappers/console/console_wrapper.h>

using namespace Vk;

const std::vector<const char*> requiredLayerNames{
#ifndef NDEBUG
	"VK_LAYER_KHRONOS_validation",
	"VK_LAYER_KHRONOS_synchronization2",
#endif // !NDEBUG
};

const std::vector<const char*> requiredInstanceExtensionNames
{
	VK_KHR_SURFACE_EXTENSION_NAME,

#ifdef WINDOWS
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif // WINDOWS
};

static std::vector<const char*> GetInstanceLayerNames()
{
	uint32_t availableLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&availableLayerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(availableLayerCount);
	vkEnumerateInstanceLayerProperties(&availableLayerCount, availableLayers.data());

#ifdef DEBUG
	Println("Layers:\n");
	for (const auto& layer : availableLayers)
	{
		Println("%s", layer.layerName);
	}
	Println("");
#endif // DEBUG

	for (auto layerName : requiredLayerNames)
	{
		// check if this layer is available
		bool layerAvailable = false;
		for (const auto& layer : availableLayers)
		{
			if (!strcmp(layerName, layer.layerName))
			{
				layerAvailable = true;
				break;
			}
		}

		if (!layerAvailable)
		{
			throw std::runtime_error(std::format("Validation layer {} required", layerName));
		}
	}

	return requiredLayerNames;
}

static std::vector<const char*> GetInstanceExtensionNames()
{
	uint32_t availableExtensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

#ifdef DEBUG
	Println("Instance Extensions:\n");
	for (const auto& extension : availableExtensions)
	{
		Println("%s v%u", extension.extensionName, extension.specVersion);
	}
	Println("");
#endif // DEBUG

	for (auto extensionName : requiredInstanceExtensionNames)
	{
		// check if this extension is available
		bool extensionAvailable = false;
		for (const auto& extension : availableExtensions)
		{
			if (!strcmp(extensionName, extension.extensionName))
			{
				extensionAvailable = true;
				break;
			}
		}

		if (!extensionAvailable)
		{
			throw std::runtime_error(std::format("Instance extension {} required", extensionName));
		}
	}

	return requiredInstanceExtensionNames;
}

void CreateInstance()
{
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Raytrace Game";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.pEngineName = "MCP Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	std::vector<const char*> enabledInstanceLayerNames = GetInstanceLayerNames();
	std::vector<const char*> enabledInstanceExtensionNames = GetInstanceExtensionNames();

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = static_cast<uint32_t>(enabledInstanceLayerNames.size());
	createInfo.ppEnabledLayerNames = enabledInstanceLayerNames.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensionNames.size());
	createInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames.data();

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}