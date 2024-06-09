#include "pch.h"
#include "vk.h"

#include <common/local_array.h>
#include <common/console/console.h>

#ifdef WINDOWS
#include <_platform/windows/window/w_window.h>
#include <_platform/windows/entrypoint/w_instance.h>
#endif // WINDOWS

#include <_wrappers/file/file_wrapper.h>
#include <_wrappers/window/window_wrapper.h>
#include <common/mat/mat4.h>

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
	VkDescriptorSet descriptorSet;
	VkDescriptorPool descriptorPool;

	VkRenderPass renderPass;

	VkDeviceMemory uniformMemory;
	VkBuffer uniformBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence renderingFence;

	std::optional<uint32_t> graphicsFamilyIndex;
	VkQueue graphicsQueue;
	VkCommandPool graphicsCommandPool;
	VkCommandBuffer graphicsCommandBuffer;

	std::optional<uint32_t> presentFamilyIndex;
	VkQueue presentQueue;
}

struct UniformInput
{
	mat4 model;
};

bool swapchainOutOfDate = false;
VkBuffer triVertexBuffer; // temp, todo: remove
VkBuffer stagingBuffer; // temp, todo: remove
VkBuffer uniformBuffer; // temp, todo: remove
VkDeviceMemory triVertexMemory; // temp, todo: remove
VkDeviceMemory stagingMemory; // temp, todo: remove
VkDeviceMemory uniformMemory; // temp, todo: remove
UniformInput* uniform;

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
		LocalArray<VkPresentModeKHR> presentModes(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(gl::physicalDevice, gl::surface, &modeCount, presentModes.data);

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
		LocalArray<VkSurfaceFormatKHR> formats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(gl::physicalDevice, gl::surface, &formatCount, formats.data);

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

			if (vkCreateImageView(gl::device, &imageInfo, nullptr, &gl::swapchainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create image view");
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

			if (vkCreateFramebuffer(gl::device, &frameBufferInfo, nullptr, &gl::swapChainFramebuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to create framebuffer");
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
	#else
		const char* enabledLayers[] = {
			"VK_LAYER_LUNARG_monitor"
		};
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
		instanceInfo.enabledLayerCount = (uint32_t)std::size(enabledLayers);
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

		// todo: profile prioritizing lower queues
	#ifdef DEBUG
		Println("Queue families:");
	#endif // DEBUG

		for (uint32_t i = 0; i < queueFamilyCount; ++i)
		{
		#ifdef DEBUG
			Println("%u:", i);
		#endif // DEBUG

			const VkQueueFamilyProperties& family = queueFamilies[i];

			if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				gl::graphicsFamilyIndex = i;

			#ifdef DEBUG
				Println("    graphics");
			#endif // DEBUG
			}

			VkBool32 present;
			vkGetPhysicalDeviceSurfaceSupportKHR(gl::physicalDevice, i, gl::surface, &present);
			if (present)
			{
				gl::presentFamilyIndex = i;
			#ifdef DEBUG
				Println("    present");
			#endif // DEBUG
			}

			if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
			#ifdef DEBUG
				Println("    compute");
			#endif // DEBUG
			}
		}
	#ifdef DEBUG
		Println();
	#endif // DEBUG

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

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo passInfo{};
		passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passInfo.attachmentCount = 1;
		passInfo.pAttachments = &colorAttachment;
		passInfo.subpassCount = 1;
		passInfo.pSubpasses = &subpass;
		passInfo.dependencyCount = 1;
		passInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(gl::device, &passInfo, nullptr, &gl::renderPass) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create render pass");
		}
	}

	CreateSwapchainEtAl();

	// Create graphics command buffer
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
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

	// Create pipeline
	{
		// Get shader code
		std::vector<char> vertCode = ReadEntireFile("core/shaders/test.vert.spv");
		std::vector<char> fragCode = ReadEntireFile("core/shaders/test.frag.spv");

		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		// Create vertex shader module
		VkShaderModuleCreateInfo vertexModuleInfo{};
		vertexModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexModuleInfo.codeSize = vertCode.size();
		vertexModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertCode.data());

		if (vkCreateShaderModule(gl::device, &vertexModuleInfo, nullptr, &vertexShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
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

		if (vkCreateShaderModule(gl::device, &fragmentModuleInfo, nullptr, &fragmentShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shader module");
		}

		VkPipelineShaderStageCreateInfo fragmentStageInfo{};
		fragmentStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragmentStageInfo.module = fragmentShaderModule;
		fragmentStageInfo.pName = "main";
		fragmentStageInfo.pSpecializationInfo = NULL;

		// Layout of vertex attributes
		VkVertexInputAttributeDescription positionAttributeDescription{};
		positionAttributeDescription.location = 0;
		positionAttributeDescription.binding = 0;
		positionAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
		positionAttributeDescription.offset = 0;

		VkVertexInputBindingDescription positionBindingDescription{};
		positionBindingDescription.binding = 0;
		positionBindingDescription.stride = 12;
		positionBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &positionBindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = &positionAttributeDescription;

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

		// Uniform input
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
		setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutInfo.bindingCount = 1;
		setLayoutInfo.pBindings = &binding;

		if (vkCreateDescriptorSetLayout(gl::device, &setLayoutInfo, nullptr, &gl::descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor set layout");
		}

		// Pipeline layout
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &gl::descriptorSetLayout;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(gl::device, &layoutInfo, nullptr, &gl::pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

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

		if (vkCreateGraphicsPipelines(gl::device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &gl::pipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline");
		}

		vkDestroyShaderModule(gl::device, vertexShaderModule, nullptr);
		vkDestroyShaderModule(gl::device, fragmentShaderModule, nullptr);
	}

	// Create sync objects
	{
		// Semaphores
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(gl::device, &semaphoreInfo, nullptr, &gl::imageAvailableSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphore");
		}

		if (vkCreateSemaphore(gl::device, &semaphoreInfo, nullptr, &gl::renderFinishedSemaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphore");
		}

		// Fences
		VkFenceCreateInfo signaledFenceInfo{};
		signaledFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		signaledFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(gl::device, &signaledFenceInfo, nullptr, &gl::renderingFence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fence");
		}
	}

#ifdef DEBUG
	// Print memory types
	{
		VkPhysicalDeviceMemoryProperties memoryProperties;
		vkGetPhysicalDeviceMemoryProperties(gl::physicalDevice, &memoryProperties);

		Println("Supported memory types:");
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
		{
			Println("%u:", i);
			Println("    Heap: %u", memoryProperties.memoryTypes[i].heapIndex);

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				Println("    Device local");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				Println("    Host visible");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
			{
				Println("    Host coherent");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)
			{
				Println("    Host cached");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)
			{
				Println("    Lazily allocated");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT)
			{
				Println("    Protected");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD)
			{
				Println("    Device coherent AMD");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD)
			{
				Println("    Device uncached AMD");
			}

			if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_RDMA_CAPABLE_BIT_NV)
			{
				Println("    RDMA capable NV");
			}
		}
		Println();
	}
#endif // DEBUG

	// Create staging buffer
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = 36;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(gl::device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer");
		}

		// Allocate mappable memory
		uint32_t memoryTypeIndex = GetMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VkMemoryAllocateInfo memInfo{};
		memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memInfo.allocationSize = 36;
		memInfo.memoryTypeIndex = memoryTypeIndex;

		if (vkAllocateMemory(gl::device, &memInfo, nullptr, &stagingMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory");
		}

		// Bind memory
		vkBindBufferMemory(gl::device, stagingBuffer, stagingMemory, 0);

		// Write vertex data
		void* loc;
		vkMapMemory(gl::device, stagingMemory, 0, 36, 0, &loc);

		float* vertData = reinterpret_cast<float*>(loc);

		vertData[0] = -1;
		vertData[1] = -1;
		vertData[2] = 0;

		vertData[3] = 1;
		vertData[4] = 1;
		vertData[5] = 0;

		vertData[6] = -1;
		vertData[7] = 1;
		vertData[8] = 0;

		vkUnmapMemory(gl::device, stagingMemory);
	}

	// Create vertex buffer
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = 36;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(gl::device, &bufferInfo, nullptr, &triVertexBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create vertex buffer");
		}

		// Allocate device local memory
		uint32_t memoryTypeIndex = GetMemoryTypeIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		VkMemoryAllocateInfo memInfo{};
		memInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memInfo.allocationSize = 36;
		memInfo.memoryTypeIndex = memoryTypeIndex;

		if (vkAllocateMemory(gl::device, &memInfo, nullptr, &triVertexMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory");
		}

		// Bind memory
		vkBindBufferMemory(gl::device, triVertexBuffer, triVertexMemory, 0);

		// Copy from staging buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkResetCommandPool(gl::device, gl::graphicsCommandPool, 0);
		vkBeginCommandBuffer(gl::graphicsCommandBuffer, &beginInfo);

		VkBufferCopy region{};
		region.srcOffset = 0;
		region.dstOffset = 0;
		region.size = 36;
		vkCmdCopyBuffer(gl::graphicsCommandBuffer, stagingBuffer, triVertexBuffer, 1, &region);

		vkEndCommandBuffer(gl::graphicsCommandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &gl::graphicsCommandBuffer;
		submitInfo.signalSemaphoreCount = 0;

		vkQueueSubmit(gl::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(gl::graphicsQueue);
	}

	// Create uniform memory and buffer
	{
		// Memory
		VkMemoryAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = sizeof(UniformInput);
		allocateInfo.memoryTypeIndex = GetMemoryTypeIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		if (vkAllocateMemory(gl::device, &allocateInfo, nullptr, &gl::uniformMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to allocate memory");
		}

		// Buffer
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(UniformInput);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(gl::device, &bufferInfo, nullptr, &gl::uniformBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create buffer");
		}

		vkBindBufferMemory(gl::device, gl::uniformBuffer, gl::uniformMemory, 0);

		void* data;
		vkMapMemory(gl::device, gl::uniformMemory, 0, sizeof(UniformInput), 0, &data);

		uniform = reinterpret_cast<UniformInput*>(data);
	}

	// Create uniform input descriptor pool and set
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.descriptorCount = 1;
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = 1;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;

		if (vkCreateDescriptorPool(gl::device, &poolInfo, nullptr, &gl::descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool");
		}

		VkDescriptorSetAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocateInfo.descriptorPool = gl::descriptorPool;
		allocateInfo.descriptorSetCount = 1;
		allocateInfo.pSetLayouts = &gl::descriptorSetLayout;

		if (vkAllocateDescriptorSets(gl::device, &allocateInfo, &gl::descriptorSet) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create descriptor pool");
		}

		// Update descriptor to point to buffer
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = gl::uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformInput);

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = gl::descriptorSet;
		write.dstBinding = 0;
		write.dstArrayElement = 0;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(gl::device, 1, &write, 0, nullptr);
	}
}

void VK_End()
{
	vkDeviceWaitIdle(gl::device);

	CleanupSwapchain();
	vkDestroyDescriptorPool(gl::device, gl::descriptorPool, nullptr);
	vkDestroyBuffer(gl::device, gl::uniformBuffer, nullptr);
	vkFreeMemory(gl::device, gl::uniformMemory, nullptr);
	vkDestroyDescriptorSetLayout(gl::device, gl::descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(gl::device, gl::pipelineLayout, nullptr);
	vkDestroyBuffer(gl::device, triVertexBuffer, nullptr);
	vkDestroyBuffer(gl::device, stagingBuffer, nullptr);
	vkDestroyBuffer(gl::device, uniformBuffer, nullptr);
	vkFreeMemory(gl::device, triVertexMemory, nullptr);
	vkFreeMemory(gl::device, stagingMemory, nullptr);
	vkFreeMemory(gl::device, uniformMemory, nullptr);
	vkDestroyFence(gl::device, gl::renderingFence, nullptr);
	vkDestroySemaphore(gl::device, gl::imageAvailableSemaphore, nullptr);
	vkDestroySemaphore(gl::device, gl::renderFinishedSemaphore, nullptr);
	vkDestroyPipeline(gl::device, gl::pipeline, nullptr);
	vkDestroyRenderPass(gl::device, gl::renderPass, nullptr);
	vkDestroyCommandPool(gl::device, gl::graphicsCommandPool, nullptr);
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
		VkResult result = vkAcquireNextImageKHR(gl::device, gl::swapchain, UINT64_MAX, gl::imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapchain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("Failed to acquire image");
		}
	}

	// Start the frame
	vkResetFences(gl::device, 1, &gl::renderingFence);

	// Start the command buffer
	VkCommandBufferBeginInfo cmdInfo{};
	cmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkResetCommandPool(gl::device, gl::graphicsCommandPool, 0);
	vkBeginCommandBuffer(gl::graphicsCommandBuffer, &cmdInfo);

	// Start the render pass
	VkRenderPassBeginInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = gl::renderPass;
	passInfo.framebuffer = gl::swapChainFramebuffers[imageIndex];
	passInfo.renderArea = { {0, 0}, gl::swapchainExtent };
	passInfo.clearValueCount = 1;
	VkClearValue clearValue = { 1, 1, 1, 1 };
	passInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(gl::graphicsCommandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Setup viewport and scissor
	VkViewport viewport{};
	viewport.x = 0;
	viewport.y = 0;
	viewport.width = (float)gl::swapchainExtent.width;
	viewport.height = (float)gl::swapchainExtent.height;
	viewport.minDepth = 0;
	viewport.maxDepth = 1;
	vkCmdSetViewport(gl::graphicsCommandBuffer, 0, 1, &viewport);

	VkRect2D scissorRect = { {0, 0}, gl::swapchainExtent };
	vkCmdSetScissor(gl::graphicsCommandBuffer, 0, 1, &scissorRect);

	// Bind pipeline
	vkCmdBindPipeline(gl::graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gl::pipeline);

	// Bind vertex buffer
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(gl::graphicsCommandBuffer, 0, 1, &triVertexBuffer, &offset);

	// Uniform
	uniform->model = mat4::Identity();
	vkCmdBindDescriptorSets(gl::graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gl::pipelineLayout, 0, 1, &gl::descriptorSet, 0, nullptr);

	// GO!
	vkCmdDraw(gl::graphicsCommandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(gl::graphicsCommandBuffer);
	vkEndCommandBuffer(gl::graphicsCommandBuffer);

	// Submit commands to queue
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &gl::imageAvailableSemaphore;
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gl::graphicsCommandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &gl::renderFinishedSemaphore;

	vkQueueSubmit(gl::graphicsQueue, 1, &submitInfo, gl::renderingFence);

	// Present and check swapchain validity
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &gl::renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &gl::swapchain;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(gl::presentQueue, &presentInfo);
		if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || swapchainOutOfDate)
		{
			RecreateSwapchain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present image");
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