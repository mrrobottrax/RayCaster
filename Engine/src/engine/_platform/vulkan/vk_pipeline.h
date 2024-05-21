#pragma once

namespace Vulkan
{
	inline VkPipelineLayout pipelineLayout;
	inline VkPipeline graphicsPipeline;
}

void CreateGraphicsPipeline();
void DestroyGraphicsPipeline();