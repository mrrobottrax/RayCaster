#pragma once

namespace Vulkan
{
	inline std::vector<VkFramebuffer> swapChainFramebuffers;
}

void CreateFrameBuffers();
void DestroyFrameBuffers();