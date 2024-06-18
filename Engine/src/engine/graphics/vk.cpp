#include "pch.h"
#include "vk.h"

#ifdef WINDOWS
#include <_platform/windows/window/w_window.h>
#include <_platform/windows/entrypoint/w_instance.h>
#endif // WINDOWS

#include <_wrappers/file/file_wrapper.h>
#include <_wrappers/window/window_wrapper.h>
#include <input/button.h>
#include <time/time.h>
#include "vulkan_error.h"
#include <input/mouse.h>
#include <player/player.h>

constexpr size_t allocationSize = 128000000;

bool swapchainOutOfDate = false;

VkResult result;

namespace gl
{
	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkExtent2D swapchainExtent;
	std::vector<VkImageView> swapchainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorSet rendererDescriptorSet;
	VkDescriptorPool rendererDescriptorPool;

	VkRenderPass renderPass;

	VkDeviceMemory deviceLocalMemory;
	VkBuffer deviceLocalBuffer;
	VkDeviceSize deviceLocalBufferNextFreeOffset = 0;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	void* stagingBufferMap;

	VkImage uChunkImage;
	VkImageView uChunkImageView;
	VkDeviceSize uChunkImageOffset;

	VkImage uTextureImage;
	VkImageView uTextureImageView;
	VkSampler uTextureSampler;
	VkDeviceSize uTextureOffset;

	VkDeviceSize uRendererInputOffset;
	VkDeviceMemory uRendererMemory;
	VkBuffer uRendererInputBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence renderingFence;

	std::optional<uint32_t> mainGraphicsFamilyIndex;
	VkQueue mainGraphicsQueue;
	VkCommandPool mainGraphicsCommandPool;
	VkCommandBuffer mainGraphicsCommandBuffer;

	std::optional<uint32_t> presentFamilyIndex;
	VkQueue presentQueue;
}

struct RendererInput
{
	alignas(16) mat4 view;
	alignas(16) uvec2 screenSize;
	alignas(16) vec3 startPos;
	alignas(4) float aspect;
};

static void CreateSwapchainEtAl()
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gl::physicalDevice, gl::surface, &capabilities);

	gl::swapchainExtent = capabilities.currentExtent;

	if (gl::swapchainExtent.width == 0 || gl::swapchainExtent.height == 0)
	{
		return;
	}

	swapchainOutOfDate = false;

	// Get available present modes
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	{
		uint32_t modeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(gl::physicalDevice, gl::surface, &modeCount, nullptr);
		std::vector<VkPresentModeKHR> presentModes(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gl::physicalDevice, gl::surface, &modeCount, presentModes.data());

		bool hasPresentMode = false;
		for (uint32_t i = 0; i < modeCount; ++i)
		{
			if (presentModes[i] == presentMode)
			{
				hasPresentMode = true;
				break;
			}
		}

		if (!hasPresentMode)
		{
			throw std::runtime_error("Surface does not support VK_PRESENT_MODE_IMMEDIATE_KHR");
		}
	}

	// Get available formats
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
	VkColorSpaceKHR colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	{
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(gl::physicalDevice, gl::surface, &formatCount, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gl::physicalDevice, gl::surface, &formatCount, formats.data());

		bool hasFormat = false;
		for (uint32_t i = 0; i < formatCount; ++i)
		{
			const VkSurfaceFormatKHR& surfaceFormat = formats[i];

			if (surfaceFormat.colorSpace == colorSpace && surfaceFormat.format == format)
			{
				hasFormat = true;
				break;
			}
		}

		if (!hasFormat)
		{
			throw std::runtime_error("Surface does not support VK_COLORSPACE_SRGB_NONLINEAR_KHR with VK_FORMAT_R8G8B8A8_SRGB");
		}
	}

	// Create swapchain
	{
		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = gl::surface;
		swapchainInfo.minImageCount = capabilities.minImageCount < capabilities.maxImageCount ? capabilities.minImageCount + 1 : capabilities.minImageCount;
		swapchainInfo.imageFormat = format;
		swapchainInfo.imageColorSpace = colorSpace;
		swapchainInfo.imageExtent = gl::swapchainExtent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
		swapchainInfo.preTransform = capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // todo: revisit
		swapchainInfo.presentMode = presentMode;
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		result = vkCreateSwapchainKHR(gl::device, &swapchainInfo, nullptr, &gl::swapchain);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create swapchain", result);
		}
	}

	// Create image views
	{
		uint32_t imageCount;
		vkGetSwapchainImagesKHR(gl::device, gl::swapchain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(gl::device, gl::swapchain, &imageCount, images.data());

		gl::swapchainImageViews.resize(imageCount);

		for (uint32_t i = 0; i < imageCount; ++i)
		{
			VkImageViewCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageInfo.image = images[i];
			imageInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			imageInfo.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			};

			VkImageSubresourceRange subRange{};
			subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subRange.baseMipLevel = 0;
			subRange.levelCount = 1;
			subRange.baseArrayLayer = 0;
			subRange.layerCount = 1;
			imageInfo.subresourceRange = subRange;

			result = vkCreateImageView(gl::device, &imageInfo, nullptr, &gl::swapchainImageViews[i]);
			if (result != VK_SUCCESS)
			{
				throw vulkan_error("Failed to create image view", result);
			}
		}
	}

	// Create framebuffers
	{
		gl::swapChainFramebuffers.resize(gl::swapchainImageViews.size());
		for (int i = 0; i < gl::swapChainFramebuffers.size(); ++i)
		{
			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = gl::renderPass;
			frameBufferInfo.attachmentCount = 1;
			frameBufferInfo.pAttachments = &gl::swapchainImageViews[i];
			frameBufferInfo.width = gl::swapchainExtent.width;
			frameBufferInfo.height = gl::swapchainExtent.height;
			frameBufferInfo.layers = 1;

			result = vkCreateFramebuffer(gl::device, &frameBufferInfo, nullptr, &gl::swapChainFramebuffers[i]);
			if (result != VK_SUCCESS)
			{
				throw vulkan_error("Failed to create framebuffer", result);
			}
		}
	}
}

static void CleanupSwapchain()
{
	vkDeviceWaitIdle(gl::device);

	vkDestroySwapchainKHR(gl::device, gl::swapchain, nullptr);
	for (size_t i = 0; i < gl::swapchainImageViews.size(); ++i)
	{
		vkDestroyImageView(gl::device, gl::swapchainImageViews[i], nullptr);
		vkDestroyFramebuffer(gl::device, gl::swapChainFramebuffers[i], nullptr);
	}
}

static void RecreateSwapchain()
{
	CleanupSwapchain();
	CreateSwapchainEtAl();
}

static uint32_t GetMemoryTypeIndex(VkMemoryPropertyFlags includeFlags)
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(gl::physicalDevice, &memoryProperties);

	std::optional<uint32_t> memoryTypeIndex;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if (memoryProperties.memoryTypes[i].propertyFlags & includeFlags)
		{
			memoryTypeIndex = i;
			break;
		}
	}

	if (!memoryTypeIndex.has_value())
	{
		throw std::runtime_error("Failed to find index of requested memory type");
	}

	return memoryTypeIndex.value();
}

static void AllocateDeviceLocalMemory(VkDeviceSize size, VkDeviceSize alignment, VkDeviceSize* pOffset, VkDeviceMemory* pDeviceMemory, VkBuffer* pBuffer)
{
	VkDeviceSize offset = ((VkDeviceSize)ceil(gl::deviceLocalBufferNextFreeOffset / (double)alignment)) * alignment;
	gl::deviceLocalBufferNextFreeOffset = offset + size;

	if (pOffset != nullptr) *pOffset = offset;
	if (pDeviceMemory != nullptr) *pDeviceMemory = gl::deviceLocalMemory;
	if (pBuffer != nullptr) *pBuffer = gl::deviceLocalBuffer;
}

void VK_Start()
{
	// Create instance
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

	#ifdef DEBUG
		// Print Layers
		Println("Available layers:");

		for (uint32_t i = 0; i < layerCount; ++i)
		{
			const VkLayerProperties& layer = layers[i];

			Println(layer.layerName);
		}
		Println();

		// Print extensions
		uint32_t extensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		Println("Instance extensions:");
		for (uint32_t i = 0; i < extensionCount; ++i)
		{
			const VkExtensionProperties& extension = extensions[i];
			Println(extension.extensionName);
		}
		Println();
	#endif // DEBUG

	#ifdef DEBUG
		// Validation layers
		const char* requestedLayers[] = {
			"VK_LAYER_KHRONOS_synchronization2",
			"VK_LAYER_KHRONOS_validation",
			"VK_LAYER_LUNARG_monitor"
		};
	#else
		const char* requestedLayers[] = {
			"VK_LAYER_LUNARG_monitor"
		};
	#endif // DEBUG

		std::vector<const char*> enabledLayers;
		enabledLayers.reserve(std::size(requestedLayers));

		for (int i = 0; i < std::size(requestedLayers); ++i)
		{
			bool hasLayer = false;
			for (int j = 0; j < layers.size(); ++j)
			{
				if (strcmp(layers[j].layerName, requestedLayers[i]) == 0)
				{
					hasLayer = true;
				}
			}

			if (hasLayer) enabledLayers.push_back(requestedLayers[i]);
		}

		// Extensions
		const char* enabledExtensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_VERSION_1_3;

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
		instanceInfo.ppEnabledLayerNames = enabledLayers.data();
		instanceInfo.enabledExtensionCount = (uint32_t)std::size(enabledExtensions);
		instanceInfo.ppEnabledExtensionNames = enabledExtensions;

		result = vkCreateInstance(&instanceInfo, nullptr, &gl::instance);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create instance", result);
		}
	}

	// Pick physical device
	{
		uint32_t deviceCount;
		vkEnumeratePhysicalDevices(gl::instance, &deviceCount, nullptr);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(gl::instance, &deviceCount, devices.data());

	#ifdef DEBUG
		Println("Scoring devices...");
	#endif // DEBUG
		std::vector<uint32_t> deviceScores(deviceCount);
		for (uint32_t i = 0; i < deviceCount; ++i)
		{
			VkPhysicalDeviceProperties properties;
			vkGetPhysicalDeviceProperties(devices[i], &properties);

			deviceScores[i] = 0;

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				deviceScores[i] += 1000;
			}

		#ifdef DEBUG
			Println("%s %u : %u", properties.deviceName, properties.deviceID, deviceScores[i]);
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

	#ifdef DEBUG
		// Print extensions
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(gl::physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(gl::physicalDevice, nullptr, &extensionCount, extensions.data());

		Println("Device extensions:");
		for (uint32_t i = 0; i < extensionCount; ++i)
		{
			Println("%s", extensions[i].extensionName);
		}
		Println();

		// Print memory types
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(gl::physicalDevice, &memoryProperties);

		Println("Supported memory types:");
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		{
			Println("%u:", i);
			Println("    Heap: %u", memoryProperties.memoryTypes[i].heapIndex);

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) Println("    Device local");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) Println("    Host visible");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) Println("    Host coherent");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) Println("    Host cached");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) Println("    Lazily allocated");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) Println("    Protected");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) Println("    Device coherent AMD");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) Println("    Device uncached AMD");
			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV) Println("    RDMA capable NV");
		}
		Println();
	#endif // DEBUG
	}

	// Create surface
	{
		VkWin32SurfaceCreateInfoKHR surfaceInfo{};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = W_Instance::hInstance;
		surfaceInfo.hwnd = W_Window::hWnd;

		result = vkCreateWin32SurfaceKHR(gl::instance, &surfaceInfo, nullptr, &gl::surface);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create win32 surface", result);
		}
	}

	// Create device
	{
		// Find queue indices
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, queueFamilies.data());

	#ifdef DEBUG
		Println("Queue families:");
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			Println("%u:", i);
			const VkQueueFamilyProperties& family = queueFamilies[i];

			VkBool32 present;
			vkGetPhysicalDeviceSurfaceSupportKHR(gl::physicalDevice, i, gl::surface, &present);

			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT) Println("    graphics");
			if (family.queueFlags & VK_QUEUE_COMPUTE_BIT) Println("    compute");
			if (family.queueFlags & VK_QUEUE_TRANSFER_BIT) Println("    transfer");
			if (family.queueFlags & VK_QUEUE_PROTECTED_BIT) Println("    protected");
			if (family.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) Println("    sparse");
			if (family.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) Println("    video decode");
			if (family.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) Println("    video encode");
			if (present) Println("    present");
		}
		Println();
	#endif // DEBUG

		// todo: profile prioritizing lower queues
		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& family = queueFamilies[i];

			VkBool32 present;
			vkGetPhysicalDeviceSurfaceSupportKHR(gl::physicalDevice, i, gl::surface, &present);

			if (!gl::mainGraphicsFamilyIndex.has_value() && family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				gl::mainGraphicsFamilyIndex = i;
			}

			if (present)
			{
				gl::presentFamilyIndex = i;
			}
		}
	#ifdef DEBUG
		Println();
		Println("Main graphics queue: %u", gl::mainGraphicsFamilyIndex.value());
		Println("Present queue: %u", gl::presentFamilyIndex.value());
		Println();
	#endif // DEBUG

		std::optional<uint32_t> queueFamilyIndices[] = { gl::mainGraphicsFamilyIndex, gl::presentFamilyIndex };
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(std::size(queueFamilyIndices));

		for (uint32_t i = 0; i < std::size(queueFamilyIndices); ++i)
		{
			if (!queueFamilyIndices[i].has_value())
			{
				throw std::runtime_error("Failed to find index of queues");
			}

			// Make sure index is unique
			bool unique = true;
			for (uint32_t j = 0; j < i; ++j)
			{
				if (queueFamilyIndices[i].value() == queueFamilyIndices[j].value())
				{
					unique = false;
					break;
				}
			}

			if (!unique) continue;

			float priority = 1;
			VkDeviceQueueCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info.queueCount = 1;
			info.queueFamilyIndex = queueFamilyIndices[i].value();
			info.pQueuePriorities = &priority;

			queueCreateInfos.push_back(info);
		}

		const char* enabledExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
		};

		VkPhysicalDeviceVulkan13Features features{};
		features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		features.synchronization2 = true;

		// Create device
		VkDeviceCreateInfo deviceInfo{};
		deviceInfo.pNext = &features;
		deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceInfo.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
		deviceInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
		deviceInfo.enabledExtensionCount = (uint32_t)std::size(enabledExtensions);
		deviceInfo.ppEnabledExtensionNames = enabledExtensions;
		deviceInfo.pEnabledFeatures = nullptr;

		vkCreateDevice(gl::physicalDevice, &deviceInfo, nullptr, &gl::device);

		// Get queues
		vkGetDeviceQueue(gl::device, gl::mainGraphicsFamilyIndex.value(), 0, &gl::mainGraphicsQueue);
		vkGetDeviceQueue(gl::device, gl::presentFamilyIndex.value(), 0, &gl::presentQueue);
	}

	{}

	// Create graphics command buffer
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
		
		result = vkCreateCommandPool(gl::device, &poolInfo, nullptr, &gl::mainGraphicsCommandPool);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create command pool", result);
		}

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = gl::mainGraphicsCommandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		result = vkAllocateCommandBuffers(gl::device, &allocateInfo, &gl::mainGraphicsCommandBuffer);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to allocate command buffers", result);
		}
	}

	// Create staging memory and buffer
	{
		// Create buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = allocationSize;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		result = vkCreateBuffer(gl::device, &bufferInfo, nullptr, &gl::stagingBuffer);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create buffer", result);
		}

		// Create memory
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		allocateInfo.allocationSize = allocationSize;

		result = vkAllocateMemory(gl::device, &allocateInfo, nullptr, &gl::stagingBufferMemory);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to allocate memory", result);
		}

		vkBindBufferMemory(gl::device, gl::stagingBuffer, gl::stagingBufferMemory, 0);

		// Map memory
		vkMapMemory(gl::device, gl::stagingBufferMemory, 0, allocationSize, 0, (void**)&gl::stagingBufferMap);
	}

	// Create device local memory and buffer
	{
		// Create buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = allocationSize;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		result = vkCreateBuffer(gl::device, &bufferInfo, nullptr, &gl::deviceLocalBuffer);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create buffer", result);
		}

		// Allocate memory
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = allocationSize;
		allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		result = vkAllocateMemory(gl::device, &allocateInfo, nullptr, &gl::deviceLocalMemory);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to allocate memory", result);
		}

		vkBindBufferMemory(gl::device, gl::deviceLocalBuffer, gl::deviceLocalMemory, 0);
	}

	// Create chunk
	{
		// Create image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_3D;
		imageInfo.format = VK_FORMAT_R8_UINT;
		imageInfo.extent = { chunkSize, chunkSize, chunkSize };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		result = vkCreateImage(gl::device, &imageInfo, nullptr, &gl::uChunkImage);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create image", result);
		}

		// Bind to memory
		VkMemoryRequirements memoryRequirements;
		VkDeviceMemory deviceMemory;
		vkGetImageMemoryRequirements(gl::device, gl::uChunkImage, &memoryRequirements);
		AllocateDeviceLocalMemory(memoryRequirements.size, memoryRequirements.alignment, &gl::uChunkImageOffset, &deviceMemory, nullptr);
		vkBindImageMemory(gl::device, gl::uChunkImage, deviceMemory, gl::uChunkImageOffset);

		// Create image view
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = 1;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = gl::uChunkImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
		viewInfo.format = VK_FORMAT_R8_UINT;
		viewInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		viewInfo.subresourceRange = subresourceRange;

		result = vkCreateImageView(gl::device, &viewInfo, nullptr, &gl::uChunkImageView);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create image view", result);
		}

		// Random blocks
		uint8_t* blocks = (uint8_t*)gl::stagingBufferMap;
		for (VkDeviceSize i = 0; i < memoryRequirements.size; ++i)
		{
			float r = (float)rand() / RAND_MAX;

			if (r > 0.95)
				blocks[i] = 1;
			else
				blocks[i] = 0;
		}

		// Copy from staging to device local
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandPool(gl::device, gl::mainGraphicsCommandPool, 0);
		vkBeginCommandBuffer(gl::mainGraphicsCommandBuffer, &beginInfo);

		// Transfer to general layout
		{
			VkImageMemoryBarrier imageMemBarrier{};
			imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemBarrier.srcAccessMask = VK_ACCESS_NONE;
			imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemBarrier.srcQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageMemBarrier.dstQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageMemBarrier.image = gl::uChunkImage;
			imageMemBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				gl::mainGraphicsCommandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				1, &imageMemBarrier
			);
		}

		// Transfer from staging buffer to image
		VkImageSubresourceLayers subresource{};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.baseArrayLayer = 0;
		subresource.layerCount = 1;

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource = subresource;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { chunkSize, chunkSize, chunkSize };

		vkCmdCopyBufferToImage(gl::mainGraphicsCommandBuffer, gl::stagingBuffer, gl::uChunkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// Transfer to general layout
		{
			VkImageMemoryBarrier imageMemBarrier{};
			imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemBarrier.dstAccessMask = VK_ACCESS_NONE;
			imageMemBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageMemBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemBarrier.srcQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageMemBarrier.dstQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageMemBarrier.image = gl::uChunkImage;
			imageMemBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				gl::mainGraphicsCommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				1, &imageMemBarrier
			);
		}

		// End and submit
		vkEndCommandBuffer(gl::mainGraphicsCommandBuffer);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &gl::mainGraphicsCommandBuffer;

		vkQueueSubmit(gl::mainGraphicsQueue, 1, &submit, VK_NULL_HANDLE);
		vkQueueWaitIdle(gl::mainGraphicsQueue);
	}

	{}

	// Create texture
	{
		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM; // todo: try srgb?

		// Create image
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = format;
		imageInfo.extent = { 16, 16, 1 };
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		result = vkCreateImage(gl::device, &imageInfo, nullptr, &gl::uTextureImage);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create image", result);
		}

		// Bind memory
		VkMemoryRequirements memoryRequirements;
		VkDeviceMemory deviceMemory;
		vkGetImageMemoryRequirements(gl::device, gl::uTextureImage, &memoryRequirements);
		AllocateDeviceLocalMemory(memoryRequirements.size, memoryRequirements.alignment, &gl::uTextureOffset, &deviceMemory, nullptr);
		vkBindImageMemory(gl::device, gl::uTextureImage, deviceMemory, gl::uTextureOffset);

		// Create image view
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.layerCount = 1;
		subresourceRange.levelCount = 1;

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = gl::uTextureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components = {
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY
		};
		viewInfo.subresourceRange = subresourceRange;

		result = vkCreateImageView(gl::device, &viewInfo, nullptr, &gl::uTextureImageView);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create image view", result);
		}

		// Create sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_NEAREST;
		samplerInfo.minFilter = VK_FILTER_NEAREST;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.mipLodBias = 0;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 0;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = 1;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;

		result = vkCreateSampler(gl::device, &samplerInfo, nullptr, &gl::uTextureSampler);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create sampler", result);
		}

		// Load image to staging buffer
		auto imageData = ReadEntireFile("data/textures/iron.ppm");
		char* stagingData = (char*)gl::stagingBufferMap;

		size_t startingIndex;
		int periodCount = 0;
		for (startingIndex = 0; startingIndex < 32; ++startingIndex)
		{
			if (periodCount == 3) break;

			if (imageData[startingIndex] == 10)
			{
				++periodCount;
			}
		}

		for (size_t i = 0; i < 256; ++i)
		{
			const size_t dataIndex = i * 3 + startingIndex;
			const size_t stagingIndex = i * 4;

			stagingData[stagingIndex] = imageData[dataIndex];
			stagingData[stagingIndex + 1] = imageData[dataIndex + 1];
			stagingData[stagingIndex + 2] = imageData[dataIndex + 2];
			stagingData[stagingIndex + 3] = 1;
		}

		// Copy image from staging buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandPool(gl::device, gl::mainGraphicsCommandPool, 0);
		vkBeginCommandBuffer(gl::mainGraphicsCommandBuffer, &beginInfo);

		// Transition image to general
		{
			VkImageMemoryBarrier imageBarrier{};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.srcAccessMask = VK_ACCESS_NONE;
			imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.srcQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageBarrier.dstQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageBarrier.image = gl::uTextureImage;
			imageBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				gl::mainGraphicsCommandBuffer,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				1, &imageBarrier
			);
		}

		// Transfer data from staging buffer
		VkImageSubresourceLayers subresourceLayers{};
		subresourceLayers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceLayers.layerCount = 1;
		subresourceLayers.mipLevel = 0;

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 16;
		region.bufferImageHeight = 16;
		region.imageSubresource = subresourceLayers;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { 16, 16, 1 };

		vkCmdCopyBufferToImage(gl::mainGraphicsCommandBuffer, gl::stagingBuffer, gl::uTextureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// Transition image to read only
		{
			VkImageMemoryBarrier imageBarrier{};
			imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			imageBarrier.dstAccessMask = VK_ACCESS_NONE;
			imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageBarrier.srcQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageBarrier.dstQueueFamilyIndex = gl::mainGraphicsFamilyIndex.value();
			imageBarrier.image = gl::uTextureImage;
			imageBarrier.subresourceRange = subresourceRange;

			vkCmdPipelineBarrier(
				gl::mainGraphicsCommandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_DEPENDENCY_BY_REGION_BIT,
				0, nullptr,
				0, nullptr,
				1, &imageBarrier
			);
		}

		// End and submit
		vkEndCommandBuffer(gl::mainGraphicsCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &gl::mainGraphicsCommandBuffer;

		vkQueueSubmit(gl::mainGraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(gl::mainGraphicsQueue);
	}

	// Get renderer input offset
	{
		AllocateDeviceLocalMemory(sizeof(RendererInput), alignof(RendererInput), &gl::uRendererInputOffset, &gl::uRendererMemory, &gl::uRendererInputBuffer);
	}

	// Create descriptor set
	{
		// Create pool
		VkDescriptorPoolSize uRendererInputSize{};
		uRendererInputSize.descriptorCount = 1;
		uRendererInputSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolSize uVolumeImageSize{};
		uVolumeImageSize.descriptorCount = 1;
		uVolumeImageSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

		VkDescriptorPoolSize uTextureSize{};
		uTextureSize.descriptorCount = 1;
		uTextureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

		VkDescriptorPoolSize sizes[] = { uRendererInputSize, uVolumeImageSize, uTextureSize };

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = (uint32_t)std::size(sizes);
		poolInfo.pPoolSizes = sizes;

		result = vkCreateDescriptorPool(gl::device, &poolInfo, nullptr, &gl::rendererDescriptorPool);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create descriptor pool", result);
		}

		// Create layout
		VkDescriptorSetLayoutBinding uRendererInputBinding{};
		uRendererInputBinding.binding = 0;
		uRendererInputBinding.descriptorCount = 1;
		uRendererInputBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uRendererInputBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding uChunkBinding{};
		uChunkBinding.binding = 1;
		uChunkBinding.descriptorCount = 1;
		uChunkBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		uChunkBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding uTextureBinding{};
		uTextureBinding.binding = 2;
		uTextureBinding.descriptorCount = 1;
		uTextureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		uTextureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkDescriptorSetLayoutBinding bindings[] = { uRendererInputBinding, uChunkBinding, uTextureBinding };

		VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
		setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutInfo.bindingCount = (uint32_t)std::size(bindings);
		setLayoutInfo.pBindings = bindings;

		result = vkCreateDescriptorSetLayout(gl::device, &setLayoutInfo, nullptr, &gl::descriptorSetLayout);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create descriptor set layout", result);
		}

		// Create set
		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = gl::rendererDescriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &gl::descriptorSetLayout;

		result = vkAllocateDescriptorSets(gl::device, &allocateInfo, &gl::rendererDescriptorSet);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create descriptor set", result);
		}

		// Update descriptor to point to buffer
		// uRendererInput
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = gl::uRendererInputBuffer;
		bufferInfo.offset = gl::uRendererInputOffset;
		bufferInfo.range = sizeof(RendererInput);

		VkWriteDescriptorSet uniformInputWrite{};
		uniformInputWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		uniformInputWrite.dstSet = gl::rendererDescriptorSet;
		uniformInputWrite.dstBinding = 0;
		uniformInputWrite.dstArrayElement = 0;
		uniformInputWrite.descriptorCount = 1;
		uniformInputWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformInputWrite.pBufferInfo = &bufferInfo;

		// uChunk
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageInfo.imageView = gl::uChunkImageView;
		imageInfo.sampler = nullptr;

		VkWriteDescriptorSet imageWrite{};
		imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		imageWrite.dstSet = gl::rendererDescriptorSet;
		imageWrite.dstBinding = 1;
		imageWrite.dstArrayElement = 0;
		imageWrite.descriptorCount = 1;
		imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		imageWrite.pImageInfo = &imageInfo;

		// uTexture
		VkDescriptorImageInfo textureImageInfo{};
		textureImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		textureImageInfo.imageView = gl::uTextureImageView;
		textureImageInfo.sampler = gl::uTextureSampler;

		VkWriteDescriptorSet textureWrite{};
		textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		textureWrite.dstSet = gl::rendererDescriptorSet;
		textureWrite.dstBinding = 2;
		textureWrite.dstArrayElement = 0;
		textureWrite.descriptorCount = 1;
		textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureWrite.pImageInfo = &textureImageInfo;

		VkWriteDescriptorSet writes[] = { uniformInputWrite, imageWrite, textureWrite };

		vkUpdateDescriptorSets(gl::device, (uint32_t)std::size(writes), writes, 0, nullptr);
	}

	{}

	// Create render pass
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		VkSubpassDependency swaphchainImageDependency{};
		swaphchainImageDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		swaphchainImageDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		swaphchainImageDependency.srcAccessMask = 0;
		swaphchainImageDependency.dstSubpass = 0;
		swaphchainImageDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		swaphchainImageDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkSubpassDependency uRendererInputDependency{};
		uRendererInputDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		uRendererInputDependency.srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		uRendererInputDependency.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		uRendererInputDependency.dstSubpass = 0;
		uRendererInputDependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		uRendererInputDependency.dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT;

		VkSubpassDependency dependencies[] = { swaphchainImageDependency, uRendererInputDependency };

		VkRenderPassCreateInfo passInfo{};
		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;
		passInfo.dependencyCount = (uint32_t)std::size(dependencies);
		passInfo.pDependencies = dependencies;

		result = vkCreateRenderPass(gl::device, &passInfo, nullptr, &gl::renderPass);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create render pass", result);
		}
	}

	// Create pipeline
	{
		// Get shader code
		std::vector<char> vertCode = ReadEntireFile("core/shaders/fullscreen.vert.spv");
		std::vector<char> fragCode = ReadEntireFile("core/shaders/renderer.frag.spv");

		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		// Create vertex shader module
		VkShaderModuleCreateInfo vertexModuleInfo{};
		vertexModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexModuleInfo.codeSize = vertCode.size();
		vertexModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertCode.data());

		result = vkCreateShaderModule(gl::device, &vertexModuleInfo, nullptr, &vertexShaderModule);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create shader module", result);
		}

		VkPipelineShaderStageCreateInfo vertexStageInfo{};
		vertexStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertexStageInfo.module = vertexShaderModule;
		vertexStageInfo.pName = "main";
		vertexStageInfo.pSpecializationInfo = NULL;

		// Create fragment shader module
		VkShaderModuleCreateInfo fragmentModuleInfo{};
		fragmentModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragmentModuleInfo.codeSize = fragCode.size();
		fragmentModuleInfo.pCode = reinterpret_cast<uint32_t*>(fragCode.data());

		result = vkCreateShaderModule(gl::device, &fragmentModuleInfo, nullptr, &fragmentShaderModule);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create shader module", result);
		}

		VkPipelineShaderStageCreateInfo fragmentStageInfo{};
		fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentStageInfo.module = fragmentShaderModule;
		fragmentStageInfo.pName = "main";
		fragmentStageInfo.pSpecializationInfo = NULL;

		// Create pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &gl::descriptorSetLayout;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		result = vkCreatePipelineLayout(gl::device, &layoutInfo, nullptr, &gl::pipelineLayout);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create pipeline layout", result);
		}

		// Layout of vertex attributes
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		// Assembly stage info
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		// Viewport info
		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = nullptr;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = nullptr;

		// Rasterization stage info
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.lineWidth = 1;

		// MSAA info
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.minSampleShading = 1;
		multisampleInfo.pSampleMask = NULL;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		// Color attachment info
		VkPipelineColorBlendAttachmentState colorAttachmentState{};
		colorAttachmentState.blendEnable = VK_FALSE;
		colorAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		// Color blending info
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendInfo.logicOpEnable = VK_FALSE;
		colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
		colorBlendInfo.attachmentCount = 1;
		colorBlendInfo.pAttachments = &colorAttachmentState;
		colorBlendInfo.blendConstants[0] = 0;
		colorBlendInfo.blendConstants[1] = 0;
		colorBlendInfo.blendConstants[2] = 0;
		colorBlendInfo.blendConstants[3] = 0;

		// Dynamic states
		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.dynamicStateCount = 2;
		dynamicInfo.pDynamicStates = dynamicStates;

		// Create pipeline
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		// todo: link time optimization flag?
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		VkPipelineShaderStageCreateInfo stages[] = { vertexStageInfo, fragmentStageInfo };
		pipelineInfo.pStages = stages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
		pipelineInfo.pTessellationState = nullptr;
		pipelineInfo.pViewportState = &viewportInfo;
		pipelineInfo.pRasterizationState = &rasterizationInfo;
		pipelineInfo.pMultisampleState = &multisampleInfo;
		pipelineInfo.pDepthStencilState = nullptr;
		pipelineInfo.pColorBlendState = &colorBlendInfo;
		pipelineInfo.pDynamicState = &dynamicInfo;
		pipelineInfo.layout = gl::pipelineLayout;
		pipelineInfo.renderPass = gl::renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;

		result = vkCreateGraphicsPipelines(gl::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &gl::pipeline);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create pipeline", result);
		}

		vkDestroyShaderModule(gl::device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(gl::device, fragmentShaderModule, nullptr);
	}

	{}

	// Create sync objects
	{
		// Semaphores
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		result = vkCreateSemaphore(gl::device, &semaphoreInfo, nullptr, &gl::imageAvailableSemaphore);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create semaphore", result);
		}

		result = vkCreateSemaphore(gl::device, &semaphoreInfo, nullptr, &gl::renderFinishedSemaphore);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create semaphore", result);
		}

		// Fences
		VkFenceCreateInfo signaledFenceInfo{};
		signaledFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		signaledFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		result = vkCreateFence(gl::device, &signaledFenceInfo, nullptr, &gl::renderingFence);
		if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to create fence", result);
		}
	}

	CreateSwapchainEtAl();
}

void VK_End()
{
	vkDeviceWaitIdle(gl::device);

	CleanupSwapchain();
	vkDestroyImage(gl::device, gl::uChunkImage, nullptr);
	vkDestroyImage(gl::device, gl::uTextureImage, nullptr);
	vkDestroyImageView(gl::device, gl::uChunkImageView, nullptr);
	vkDestroyImageView(gl::device, gl::uTextureImageView, nullptr);
	vkDestroySampler(gl::device, gl::uTextureSampler, nullptr);
	vkDestroyDescriptorPool(gl::device, gl::rendererDescriptorPool, nullptr);
	vkDestroyBuffer(gl::device, gl::deviceLocalBuffer, nullptr);
	vkDestroyBuffer(gl::device, gl::stagingBuffer, nullptr);
	vkFreeMemory(gl::device, gl::deviceLocalMemory, nullptr);
	vkFreeMemory(gl::device, gl::stagingBufferMemory, nullptr);
	vkDestroyDescriptorSetLayout(gl::device, gl::descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(gl::device, gl::pipelineLayout, nullptr);
	vkDestroyFence(gl::device, gl::renderingFence, nullptr);
	vkDestroySemaphore(gl::device, gl::imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(gl::device, gl::renderFinishedSemaphore, nullptr);
	vkDestroyPipeline(gl::device, gl::pipeline, nullptr);
	vkDestroyRenderPass(gl::device, gl::renderPass, nullptr);
	vkDestroyCommandPool(gl::device, gl::mainGraphicsCommandPool, nullptr);
	vkDestroySurfaceKHR(gl::instance, gl::surface, nullptr);
	vkDestroyDevice(gl::device, nullptr);
	vkDestroyInstance(gl::instance, nullptr);
}

void VK_Frame()
{
	vkWaitForFences(gl::device, 1, &gl::renderingFence, VK_TRUE, UINT64_MAX);

	// Don't render while minimized
	if (gl::swapchainExtent.width == 0 || gl::swapchainExtent.height == 0)
	{
		CreateSwapchainEtAl();
		return;
	}

	// Get image index and check swapchain validity
	uint32_t imageIndex;
	{
		result = vkAcquireNextImageKHR(gl::device, gl::swapchain, UINT64_MAX, gl::imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw vulkan_error("Failed to acquire image", result);
		}
	}

	// Update camera
	RendererInput& uRendererInput = *(RendererInput*)gl::stagingBufferMap;
	uRendererInput.view = mat4::InverseTransformation(camPos, camRot.x, camRot.y, 0);
	uRendererInput.startPos = camPos;
	uRendererInput.screenSize.x = gl::swapchainExtent.width;
	uRendererInput.screenSize.y = gl::swapchainExtent.height;
	uRendererInput.aspect = float(gl::swapchainExtent.width) / gl::swapchainExtent.height;

	// Start the frame
	vkResetFences(gl::device, 1, &gl::renderingFence);

	// Start the command buffer
	VkCommandBufferBeginInfo cmdInfo{};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkResetCommandPool(gl::device, gl::mainGraphicsCommandPool, 0);
	vkBeginCommandBuffer(gl::mainGraphicsCommandBuffer, &cmdInfo);

	// Copy uRendererInput from staging buffer
	VkBufferCopy copy{};
	copy.size = sizeof(RendererInput);
	copy.dstOffset = gl::uRendererInputOffset;

	vkCmdCopyBuffer(gl::mainGraphicsCommandBuffer, gl::stagingBuffer, gl::uRendererInputBuffer, 1, &copy);

	// Start the render pass
	VkRenderPassBeginInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = gl::renderPass;
	passInfo.framebuffer = gl::swapChainFramebuffers[imageIndex];
	passInfo.renderArea = { {0, 0}, gl::swapchainExtent };
	passInfo.clearValueCount = 1;
	VkClearValue clearValue = { 0.53f, 0.81f, 0.92f, 1 };
	passInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(gl::mainGraphicsCommandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Setup viewport and scissor
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)gl::swapchainExtent.width;
	viewport.height = (float)gl::swapchainExtent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(gl::mainGraphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissorRect = { {0, 0}, gl::swapchainExtent };
	vkCmdSetScissor(gl::mainGraphicsCommandBuffer, 0, 1, &scissorRect);

	// Bind pipeline
	vkCmdBindPipeline(gl::mainGraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gl::pipeline);

	// Uniforms
	vkCmdBindDescriptorSets(gl::mainGraphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gl::pipelineLayout, 0, 1, &gl::rendererDescriptorSet, 0, nullptr);

	// GO!
	vkCmdDraw(gl::mainGraphicsCommandBuffer, 6, 1, 0, 0);

	vkCmdEndRenderPass(gl::mainGraphicsCommandBuffer);
	vkEndCommandBuffer(gl::mainGraphicsCommandBuffer);

	// Submit commands to queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &gl::imageAvailableSemaphore;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gl::mainGraphicsCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &gl::renderFinishedSemaphore;

	vkQueueSubmit(gl::mainGraphicsQueue, 1, &submitInfo, gl::renderingFence);

	// Present and check swapchain validity
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &gl::renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &gl::swapchain;
		presentInfo.pImageIndices = &imageIndex;

		result = vkQueuePresentKHR(gl::presentQueue, &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || swapchainOutOfDate)
		{
			RecreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw vulkan_error("Failed to present image", result);
		}
	}
}

void VK_Resize()
{
	uint32_t width, height;
	GetMainWindowSize(&width, &height);

	if (width != gl::swapchainExtent.width || height != gl::swapchainExtent.height)
	{
		swapchainOutOfDate = true;
	}
}