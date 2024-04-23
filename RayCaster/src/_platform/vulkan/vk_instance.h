#pragma once

namespace Vk
{
	inline VkInstance instance;
}

void CreateInstance();

std::vector<const char*> GetInstanceLayerNames();
std::vector<const char*> GetInstanceExtensionNames();