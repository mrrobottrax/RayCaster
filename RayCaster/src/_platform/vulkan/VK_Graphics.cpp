#include "pch.h"
#include "vk_graphics.h"

#include <_wrappers/console/console_wrapper.h>

using namespace VK;

void VK_Start()
{
	CreateInstance();
	CreateDevice();

	// vkCreateWin32SurfaceKHR();
}

void VK_End()
{
	vkDestroyInstance(instance, nullptr);
}

void VK::CreateInstance()
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

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create instance!");
	}
}

void VK::CreateDevice()
{
	physicalDevice = PickPhysicalDevice();
}

// Get a vector of all enabled layer names
std::vector<const char*> VK::GetEnabledInstanceLayerNames()
{
	uint32_t availableLayersCount = 0;
	vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);

	auto availableLayers = std::vector<VkLayerProperties>(availableLayersCount);
	vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

#ifdef _DEBUG
	Println("Layers:\n");
	for (const auto& layer : availableLayers)
	{
		Println("%s", layer.layerName);
	}
	Println("");
#endif // _DEBUG

	auto requiredLayerNames = std::vector<const char*>();

#ifndef NDEBUG
	requiredLayerNames.push_back("VK_LAYER_KHRONOS_validation");
	requiredLayerNames.push_back("VK_LAYER_KHRONOS_synchronization2");
#endif // !NDEBUG

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
			throw std::runtime_error(std::format("Validation layer {} required but not available", layerName));
		}
	}

	return requiredLayerNames;
}

VkPhysicalDevice VK::PickPhysicalDevice()
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	auto physicalDevices = std::vector<VkPhysicalDevice>(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : physicalDevices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

		int rating = RatePhysicalDeviceSuitability(properties);
		candidates.insert(std::make_pair(rating, device));
	}

#ifdef _DEBUG
	Println("Devices:\n");

	for (const auto& pair : candidates)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(pair.second, &properties);

		Println("%s : %u", properties.deviceName, pair.first);
	}

	Println("");
#endif // _DEBUG

	return candidates.rbegin()->second;
}
int VK::RatePhysicalDeviceSuitability(const VkPhysicalDeviceProperties& properties)
{
	if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return 0;
	}

	return 1;
}