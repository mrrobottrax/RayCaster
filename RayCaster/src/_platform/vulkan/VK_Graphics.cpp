#include "pch.h"
#include "vk_graphics.h"

#include "vk.h"
#include <_wrappers/console/console_wrapper.h>

#ifdef WINDOWS
#include "windows/vk_w_window.h"
#endif // WINDOWS

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

void CreateDevice()
{
	physicalDevice = PickPhysicalDevice();

	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies{indices.graphicsFamily.value(), indices.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	std::vector<const char*> enabledExtensions = GetDeviceExtensionNames(physicalDevice);

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.pEnabledFeatures = nullptr;

	vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void CreateSurface()
{
#ifdef WINDOWS
	VK_W_CreateSurface();
#endif // WINDOWS
}

std::vector<const char*> GetInstanceLayerNames()
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
			throw std::runtime_error(std::format("Validation layer {} required", layerName));
		}
	}

	return requiredLayerNames;
}

std::vector<const char*> GetInstanceExtensionNames()
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

	std::vector<const char*> requiredExtensionNames;

	requiredExtensionNames.push_back("VK_KHR_surface");
#ifdef WINDOWS
	requiredExtensionNames.push_back("VK_KHR_win32_surface");
#endif // WINDOWS

	for (auto extensionName : requiredExtensionNames)
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

	return requiredExtensionNames;
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
	// todo: check if device supports required vulkan version

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

		VkBool32 presentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported);

		if (presentSupported)
		{
			indices.presentFamily = i;
		}

		if (indices.IsComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}

std::vector<const char*> GetDeviceExtensionNames(VkPhysicalDevice physicalDevice)
{
	uint32_t availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

#ifdef DEBUG
	Println("Device Extensions:\n");
	for (const auto& extension : availableExtensions)
	{
		Println("%s v%u", extension.extensionName, extension.specVersion);
	}
	Println("");
#endif // DEBUG

	std::vector<const char*> requiredExtensionNames;

	/*requiredExtensionNames.push_back("VK_KHR_surface");
	requiredExtensionNames.push_back("VK_KHR_win32_surface");*/

	for (auto extensionName : requiredExtensionNames)
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
			throw std::runtime_error(std::format("Device extension {} required", extensionName));
		}
	}

	return requiredExtensionNames;
}