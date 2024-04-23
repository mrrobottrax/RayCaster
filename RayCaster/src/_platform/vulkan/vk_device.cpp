#include "pch.h"
#include "vk_device.h"

#include "vk.h"
#include <_wrappers/console/console_wrapper.h>

using namespace Vk;
using namespace Vk::Queue;

void CreateDevice()
{
	physicalDevice = PickPhysicalDevice();

	QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice);

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
	// todo: check if device supports needed extensions

	if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return false;
	}

	QueueFamilyIndices indices = FindQueueFamilyIndices(device);
	if (!indices.IsComplete())
	{
		return false;
	}

	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
	{
		return false;
	}

	return true;
}

QueueFamilyIndices FindQueueFamilyIndices(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupported);

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

std::vector<const char*> GetDeviceExtensionNames(VkPhysicalDevice device)
{
	uint32_t availableExtensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions.data());

#ifdef DEBUG
	Println("Device Extensions:\n");
	for (const auto& extension : availableExtensions)
	{
		Println("%s v%u", extension.extensionName, extension.specVersion);
	}
	Println("");
#endif // DEBUG

	std::vector<const char*> requiredExtensionNames{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

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

SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}