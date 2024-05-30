#include "pch.h"
#include "vk.h"

#include <common/local_array.h>
#include <common/console/console.h>

#ifdef WINDOWS
#include <_platform/windows/window/w_window.h>
#include <_platform/windows/entrypoint/w_instance.h>
#endif // WINDOWS
#include <_wrappers/file/file_wrapper.h>

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

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkSemaphore semaphore;
	VkFence renderingFence;

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

	// Create framebuffers
	{
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
				throw std::runtime_error("Failed to create framebuffer");
			}
		}
	}

	// Create pipeline
	{
		std::vector<char> vertCode = ReadEntireFile("core/shaders/test.vert.spv");
		std::vector<char> fragCode = ReadEntireFile("core/shaders/test.frag.spv");

		VkShaderModule vertexShaderModule;
		VkShaderModule fragmentShaderModule;

		// Vertex stage
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

		// Fragment stage
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

		// Pipeline
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportInfo{};
		viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportInfo.viewportCount = 1;
		viewportInfo.pViewports = nullptr;
		viewportInfo.scissorCount = 1;
		viewportInfo.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizationInfo.depthBiasEnable = VK_FALSE;
		rasterizationInfo.lineWidth = 1;

		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampleInfo.sampleShadingEnable = VK_FALSE;
		multisampleInfo.minSampleShading = 1;
		multisampleInfo.pSampleMask = NULL;
		multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		multisampleInfo.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorAttachmentState{};
		colorAttachmentState.blendEnable = VK_FALSE;
		colorAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		colorAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

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

		VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicInfo{};
		dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicInfo.dynamicStateCount = 2;
		dynamicInfo.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 0;
		layoutInfo.pSetLayouts = nullptr;
		layoutInfo.pushConstantRangeCount = 0;
		layoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(gl::device, &layoutInfo, nullptr, &gl::pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

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

	// Sync objects
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		if (vkCreateSemaphore(gl::device, &semaphoreInfo, nullptr, &gl::semaphore) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create semaphore");
		}

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		if (vkCreateFence(gl::device, &fenceInfo, nullptr, &gl::renderingFence) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create fence");
		}
	}
}

void VK_End()
{
	vkDeviceWaitIdle(gl::device);

	vkDestroyFence(gl::device, gl::renderingFence, nullptr);
	vkDestroySemaphore(gl::device, gl::semaphore, nullptr);
	vkDestroyPipeline(gl::device, gl::pipeline, nullptr);
	vkDestroyPipelineLayout(gl::device, gl::pipelineLayout, nullptr);
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
	vkWaitForFences(gl::device, 1, &gl::renderingFence, VK_TRUE, UINT64_MAX);
	vkResetFences(gl::device, 1, &gl::renderingFence);

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(gl::device, gl::swapchain, UINT64_MAX, gl::semaphore, VK_NULL_HANDLE, &imageIndex);

	// Begin recording
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(gl::graphicsCommandBuffer, &beginInfo);

	vkCmdBindPipeline(gl::graphicsCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gl::pipeline);

	// Set viewport and scissor
	{
		VkViewport viewport{};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)gl::swapChainExtent.width;
		viewport.height = (float)gl::swapChainExtent.height;
		viewport.minDepth = 0;
		viewport.maxDepth = 1;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = gl::swapChainExtent;

		vkCmdSetViewport(gl::graphicsCommandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(gl::graphicsCommandBuffer, 0, 1, &scissor);
	}

	VkRenderPassBeginInfo passInfo{};
	passInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	passInfo.renderPass = gl::renderPass;
	passInfo.framebuffer = gl::framebuffers[imageIndex];
	passInfo.renderArea = { {0, 0}, gl::swapChainExtent };
	VkClearValue clear{ 0, 0, 0, 1 };
	passInfo.clearValueCount = 1;
	passInfo.pClearValues = &clear;

	vkCmdBeginRenderPass(gl::graphicsCommandBuffer, &passInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdDraw(gl::graphicsCommandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(gl::graphicsCommandBuffer);

	// End recording
	vkEndCommandBuffer(gl::graphicsCommandBuffer);

	VkPipelineStageFlags flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &gl::semaphore;
	submitInfo.pWaitDstStageMask = &flags;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &gl::graphicsCommandBuffer;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;

	if (vkQueueSubmit(gl::graphicsQueue, 1, &submitInfo, gl::renderingFence) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to submit queue");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 0;
	presentInfo.pWaitSemaphores = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &gl::swapchain;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL;

	if (vkQueuePresentKHR(gl::presentQueue, &presentInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to present swapchain");
	}

	imageIndex = (imageIndex + 1) % (int)gl::framebuffers.size();
}

void VK_Resize()
{

}