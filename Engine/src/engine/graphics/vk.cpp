#include "pch.h"
#include "vk.h"

#include <common/local_array.h>
#include <common/console/console.h>

#ifdef WINDOWS
#include <_platform/windows/window/w_window.h>
#include <_platform/windows/entrypoint/w_instance.h>
#endif // WINDOWS

namespace gl
{
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;

	VkRenderPass renderPass;

	std::optional<uint32_t> graphicsFamilyIndex;
	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsCommandBuffer;

	std::optional<uint32_t> presentFamilyIndex;
	VkQueue presentQueue;
	VkCommandPool presentCommandPool;
	VkCommandBuffer presentCommandBuffer;
}

void VK_Start()
{
	// Create instance
	{
	#ifdef DEBUG
		// Print Layers
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

		// Print extensions
		uint32_t extensionCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		LocalArray<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data);

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

		// Extensions
		const char* enabledExtensions[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		};

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);

		VkInstanceCreateInfo instanceInfo{};
		instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceInfo.pApplicationInfo = &appInfo;
		instanceInfo.enabledLayerCount = enabledLayerCount;
		instanceInfo.ppEnabledLayerNames = enabledLayers;
		instanceInfo.enabledExtensionCount = (uint32_t)std::size(enabledExtensions);
		instanceInfo.ppEnabledExtensionNames = enabledExtensions;

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

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
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

	#ifdef DEBUG
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(gl::physicalDevice, nullptr, &extensionCount, nullptr);
		LocalArray<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(gl::physicalDevice, nullptr, &extensionCount, extensions.data);

		Println("Device extensions:");
		for (uint32_t i = 0; i < extensionCount; ++i)
		{
			Println("%s", extensions[i].extensionName);
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

		if (vkCreateWin32SurfaceKHR(gl::instance, &surfaceInfo, nullptr, &gl::surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create win32 surface");
		}
	}

	// Create device
	{
		// Find queue indices
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, nullptr);
		LocalArray<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gl::physicalDevice, &queueFamilyCount, queueFamilies.data);

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
			const VkQueueFamilyProperties& family = queueFamilies[i];

			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				gl::graphicsFamilyIndex = i;
			}

			VkBool32 present;
			vkGetPhysicalDeviceSurfaceSupportKHR(gl::physicalDevice, i, gl::surface, &present);
			if (present)
			{
				gl::presentFamilyIndex = i;
			}
		}

		if (!gl::graphicsFamilyIndex.has_value() || !gl::presentFamilyIndex.has_value())
		{
			throw std::runtime_error("Failed to find index of queues");
		}

		uint32_t queueFamilyIndices[] = { gl::graphicsFamilyIndex.value(), gl::presentFamilyIndex.value() };
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(std::size(queueFamilyIndices));

		for (uint32_t i = 0; i < std::size(queueFamilyIndices); ++i)
		{
			// Make sure index is unique
			bool unique = true;
			for (uint32_t j = 0; j < i; ++j)
			{
				if (queueFamilyIndices[i] == queueFamilyIndices[j])
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
			info.queueFamilyIndex = queueFamilyIndices[i];
			info.pQueuePriorities = &priority;

			queueCreateInfos.push_back(info);
		}

		const char* enabledExtensions[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};

		// Create device
		VkDeviceCreateInfo deviceInfo{};
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
		vkGetDeviceQueue(gl::device, gl::graphicsFamilyIndex.value(), 0, &gl::graphicsQueue);
		vkGetDeviceQueue(gl::device, gl::presentFamilyIndex.value(), 0, &gl::presentQueue);
	}

	// Create swapchain
	{

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gl::physicalDevice, gl::surface, &capabilities);

		gl::swapChainExtent = capabilities.currentExtent;

		VkSwapchainCreateInfoKHR swapchainInfo{};
		swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainInfo.surface = gl::surface;
		swapchainInfo.minImageCount = capabilities.minImageCount < capabilities.maxImageCount ? capabilities.minImageCount + 1 : capabilities.minImageCount;
		swapchainInfo.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
		swapchainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		swapchainInfo.imageExtent = gl::swapChainExtent;
		swapchainInfo.imageArrayLayers = 1;
		swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		uint32_t queueIndices[] = { gl::graphicsFamilyIndex.value(), gl::presentFamilyIndex.value() };
		swapchainInfo.queueFamilyIndexCount = (uint32_t)std::size(queueIndices);
		swapchainInfo.pQueueFamilyIndices = queueIndices;
		swapchainInfo.preTransform = capabilities.currentTransform;
		swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // todo: revisit
		swapchainInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; // vsync
		swapchainInfo.clipped = VK_TRUE;
		swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(gl::device, &swapchainInfo, nullptr, &gl::swapchain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain");
		}
	}

	// Create image views
	{
		uint32_t imageCount;
		vkGetSwapchainImagesKHR(gl::device, gl::swapchain, &imageCount, nullptr);
		LocalArray<VkImage> images(imageCount);
		vkGetSwapchainImagesKHR(gl::device, gl::swapchain, &imageCount, images.data);

		gl::imageViews.resize(imageCount);

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

			if (vkCreateImageView(gl::device, &imageInfo, nullptr, &gl::imageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image view");
			}
		}
	}

	// Graphics command buffer
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = gl::graphicsFamilyIndex.value();

		if (vkCreateCommandPool(gl::device, &poolInfo, nullptr, &gl::graphicsCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool");
		}

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = gl::graphicsCommandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(gl::device, &allocateInfo, &gl::graphicsCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers");
		}
	}

	// Present command buffer
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = gl::presentFamilyIndex.value();

		if (vkCreateCommandPool(gl::device, &poolInfo, nullptr, &gl::presentCommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create command pool");
		}

		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.commandPool = gl::presentCommandPool;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocateInfo.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(gl::device, &allocateInfo, &gl::presentCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate command buffers");
		}
	}

	// Create render pass
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentReference{};
		colorAttachmentReference.attachment = 0;
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentReference;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

		VkRenderPassCreateInfo passInfo{};
		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;
		passInfo.dependencyCount = 0;
		passInfo.pDependencies = nullptr;

		if (vkCreateRenderPass(gl::device, &passInfo, nullptr, &gl::renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass");
		}
	}

	gl::framebuffers.resize(gl::imageViews.size());
	for (int i = 0; i < gl::framebuffers.size(); ++i)
	{
		VkFramebufferCreateInfo frameBufferInfo{};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = gl::renderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = &gl::imageViews[i];
		frameBufferInfo.width = gl::swapChainExtent.width;
		frameBufferInfo.height = gl::swapChainExtent.height;
		frameBufferInfo.layers = 1;

		if (vkCreateFramebuffer(gl::device, &frameBufferInfo, nullptr, &gl::framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass");
		}
	}
}

void VK_End()
{
	vkDestroyRenderPass(gl::device, gl::renderPass, nullptr);
	vkDestroyCommandPool(gl::device, gl::graphicsCommandPool, nullptr);
	vkDestroyCommandPool(gl::device, gl::presentCommandPool, nullptr);
	for (size_t i = 0; i < gl::imageViews.size(); ++i)
	{
		vkDestroyImageView(gl::device, gl::imageViews[i], nullptr);
		vkDestroyFramebuffer(gl::device, gl::framebuffers[i], nullptr);
	}
	vkDestroySwapchainKHR(gl::device, gl::swapchain, nullptr);
	vkDestroySurfaceKHR(gl::instance, gl::surface, nullptr);
	vkDestroyDevice(gl::device, nullptr);
	vkDestroyInstance(gl::instance, nullptr);
}

void VK_Frame()
{
	/*VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(gl::graphicsCommandBuffer, &beginInfo);

	VkRenderPassBeginInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = gl::renderPass;
	passInfo.framebuffer = gl::framebuffer;
	passInfo.renderArea = gl::framebuffer.dimensions;
	passInfo.clearValueCount = 1;
	VkClearValue clear{0, 0, 0, 0};
	passInfo.pClearValues = &clear;

	vkCmdBeginRenderPass(gl::graphicsCommandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdEndRenderPass(gl::graphicsCommandBuffer);

	vkCmdDraw(gl::graphicsCommandBuffer, 3, 1, 0, 0);

	vkEndCommandBuffer(gl::graphicsCommandBuffer);*/
}

void VK_Resize()
{

}