#include "pch.h"
#include "vk_device.h"

#include "vk.h"
#include "vk_queue.h"
#include "vk_swapchain.h"
#include "vk_instance.h"

#include "_wrappers/console/console_wrapper.h"

using namespace Vulkan;
using namespace Vulkan::Queue;

const std::vector<const char*> requiredDeviceExtensions{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static bool GetDeviceExtensionSupport(VkPhysicalDevice device)
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

	for (auto extensionName : requiredDeviceExtensions)
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
			Println("Extension %s required", extensionName);
			return false;
		}
	}

	return true;
}

static bool IsDeviceSuitable(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties)
{
	// todo: check if device supports required vulkan version

	if (properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) // todo: add integrated gpu support
	{
		return false;
	}

	return QueueFamiliesAdequate(device) && GetDeviceExtensionSupport(device) && SwapChainAdequate(device);
}

static int RatePhysicalDeviceSuitability(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties)
{
	if (!IsDeviceSuitable(device, properties))
	{
		return 0;
	}

	return 1;
}

static VkPhysicalDevice PickPhysicalDevice()
{
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

	std::multimap<int, VkPhysicalDevice> candidates;

#ifdef DEBUG
	Println("Devices:\n");
#endif // DEBUG

	for (const auto& device : physicalDevices)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(device, &properties);

	#ifdef DEBUG
		Println("%s", properties.deviceName);
	#endif // DEBUG

		int rating = RatePhysicalDeviceSuitability(device, properties);
		candidates.insert(std::make_pair(rating, device));

	#ifdef DEBUG
		Print("%s: score: %u ", properties.deviceName, rating);
		Print("version: %u : %u.%u.%u", VK_API_VERSION_VARIANT(properties.apiVersion), VK_API_VERSION_MAJOR(properties.apiVersion), VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));
		Println("");
	#endif // DEBUG
	}

	if (candidates.rbegin()->first <= 0)
	{
		throw std::runtime_error("Failed to find suitable device");
	}

	return candidates.rbegin()->second;
}

void CreateDevice()
{
	physicalDevice = PickPhysicalDevice();

	QueueFamilyIndices indices = FindQueueFamilyIndices(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(), indices.presentFamily.value() };

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

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
	createInfo.pEnabledFeatures = nullptr;

	vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}