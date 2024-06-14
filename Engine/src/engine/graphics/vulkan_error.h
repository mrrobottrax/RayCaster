#pragma once

class vulkan_error : public std::exception
{
public:
	explicit vulkan_error(const std::string& _Message, VkResult _Result) : std::exception((_Message + ":" + string_VkResult(_Result)).c_str())
	{}

	explicit vulkan_error(const char* _Message, VkResult _Result) : vulkan_error(std::string(_Message), _Result)
	{}
};