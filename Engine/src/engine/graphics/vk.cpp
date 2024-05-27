#include "pch.h"
#include "vk.h"

#include <common/local_array.h>
#include <common/console/console.h>

namespace gl
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue graphicsQueue;
}

static void PrintLayers()
{
	Println("Available layers:");

	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	LocalArray<VkLayerProperties> layers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layers.data);

	for (uint32_t i = 0; i < layerCount; ++i)
	{
		const VkLayerProperties& layer = layers[i];

		Println(layer.layerName);
	}
	Println();
}

void VK_Start()
{
	// Create instance
	{
	#ifdef DEBUG
		PrintLayers();

		const char* enabledLayers[] = {
			"VK_LAYER_KHRONOS_synchronization2",
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor",
		};
		uint32_t enabledLayerCount = (uint32_t)std::size(enabledLayers);
	#else
		const char* const* enabledLayers = nullptr;
		uint32_t enabledLayerCount = 0;
	#endif // DEBUG

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = enabledLayerCount;
		instanceInfo.ppEnabledLayerNames = enabledLayers;
		instanceInfo.enabledExtensionCount = 0;
		instanceInfo.ppEnabledExtensionNames = nullptr;

		if (vkCreateInstance(&instanceInfo, nullptr, &gl::instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create instance");
		}
	}

	// Pick physical device
	{
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(gl::instance, &deviceCount, nullptr);

		LocalArray<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(gl::instance, &deviceCount, devices.data);

	#ifdef DEBUG
		Println("Scoring devices...");
	#endif // DEBUG
		LocalArray<uint32_t> deviceScores(deviceCount);
		for (uint32_t i = 0; i < deviceCount; ++i)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(devices[i], &properties);

			deviceScores[i] = 0;

			if (properties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				deviceScores[i] += 1000;
			}

		#ifdef DEBUG
			Println("%s : %u", properties.deviceName, deviceScores[i]);
		#endif // DEBUG
		}
	#ifdef DEBUG
		Println();
	#endif // DEBUG

		// Pick winner (todo: sort)
		uint32_t bestScore = 0;
		VkPhysicalDevice bestDevice{};
		for (uint32_t i = 0; i < deviceCount; ++i)
		{
			if (deviceScores[i] > bestScore)
			{
				bestScore = deviceScores[i];
				bestDevice = devices[i];
			}
		}

		if (bestScore == 0)
		{
			throw std::runtime_error("No suitable device found");
		}

		gl::physicalDevice = bestDevice;
	}

	// Create device
	{
		// Find queue indices
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, nullptr);
		LocalArray<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, queueFamilies.data);

		std::optional<uint32_t> graphicsFamilyIndex;

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& family = queueFamilies[i];

			if (family.queueFlags && VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			{
				graphicsFamilyIndex = i;
			}
		}

		if (!graphicsFamilyIndex.has_value())
		{
			throw std::runtime_error("Failed to find index of queues");
		}

		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueCount = 1;
		queueInfo.queueFamilyIndex = graphicsFamilyIndex.value();
		float priority = 1;
		queueInfo.pQueuePriorities = &priority;

		// Create device
		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = 1;
		deviceInfo.pQueueCreateInfos = &queueInfo;
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = 0;
		deviceInfo.ppEnabledExtensionNames = nullptr;
		deviceInfo.pEnabledFeatures = nullptr;

		vkCreateDevice(gl::physicalDevice, &deviceInfo, nullptr, &gl::device);

		// Get queues
		vkGetDeviceQueue(gl::device, graphicsFamilyIndex.value(), 0, &gl::graphicsQueue);
	}
}

void VK_End()
{
	vkDestroyDevice(gl::device, nullptr);
	vkDestroyInstance(gl::instance, nullptr);
}

void VK_Frame()
{

}

void VK_Resize()
{

}