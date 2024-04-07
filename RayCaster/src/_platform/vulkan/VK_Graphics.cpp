#include "pch.h"
#include "vk_graphics.h"

#include <_wrappers/console/console_wrapper.h>

using namespace VK;

void CreateInstance()
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

void CreateDevice()
{
	physicalDevice = PickPhysicalDevice();

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.enabledExtensionCount = 0;
	createInfo.ppEnabledExtensionNames = nullptr;
	createInfo.pEnabledFeatures = nullptr;

	vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}

// Get a vector of all enabled layer names
std::vector<const char*> GetEnabledInstanceLayerNames()
{
	uint32_t availableLayersCount = 0;
	vkEnumerateInstanceLayerProperties(&availableLayersCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(availableLayersCount);
	vkEnumerateInstanceLayerProperties(&availableLayersCount, availableLayers.data());

#ifdef DEBUG
	Println("Layers:\n");
	for (const auto& layer : availableLayers)
	{
		Println("%s", layer.layerName);
	}
	Println("");
#endif // DEBUG

	std::vector<const char*> requiredLayerNames;

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

VkPhysicalDevice PickPhysicalDevice()
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

	for (const auto& device : physicalDevices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

		int rating = RatePhysicalDeviceSuitability(device, properties);
		candidates.insert(std::make_pair(rating, device));
	}

#ifdef DEBUG
	Println("Devices:\n");

	for (const auto& pair : candidates)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(pair.second, &properties);

		Print("%s score: %u ", properties.deviceName, pair.first);
		Print("version: %u : %u.%u.%u", VK_API_VERSION_VARIANT(properties.apiVersion), VK_API_VERSION_MAJOR(properties.apiVersion), VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));
		Println("");
	}

	Println("");
#endif // DEBUG

	if (candidates.rbegin()->first <= 0)
	{
		throw std::runtime_error("Failed to find suitable device");
	}

	return candidates.rbegin()->second;
}

int RatePhysicalDeviceSuitability(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties)
{
	if (!IsDeviceSuitable(device, properties))
	{
		return 0;
	}

	return 1;
}

bool IsDeviceSuitable(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties)
{
	if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return false;
	}

	QueueFamilyIndices indices = FindQueueFamilies(device);

	if (!indices.IsComplete())
	{
		return false;
	}

	return true;
}

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}