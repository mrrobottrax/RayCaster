#pragma once

namespace Vulkan
{
	inline std::vector<VkFramebuffer> swapChainFramebuffers;
	inline bool framebufferResized = false;
}

void CreateFrameBuffers();
void DestroyFrameBuffers();