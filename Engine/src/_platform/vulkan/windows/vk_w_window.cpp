#include "core_pch.h"
#include "vk_w_window.h"

#include <_platform/vulkan/vk.h>
#include <_platform/vulkan/vk_instance.h>
#include <_platform/vulkan/vk_surface.h>

#include <_platform/windows/main/win_instance.h>
#include <_platform/windows/window/w_window.h>

void VK_W_CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = W_Instance::hInstance;
	createInfo.hwnd = W_Window::hWnd;

	if (vkCreateWin32SurfaceKHR(Vk::instance, &createInfo, nullptr, &Vk::surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create win32 surface");
	}
}