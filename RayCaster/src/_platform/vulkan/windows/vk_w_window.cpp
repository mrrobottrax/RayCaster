#include "pch.h"
#include "vk_w_window.h"

#include <_platform/vulkan/vk_graphics.h>
#include <_platform/windows/main/win_main.h>
#include <_platform/windows/window/w_window.h>

using namespace VK;

void VK_W_CreateSurface()
{
	VkWin32SurfaceCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = W_Instance::hInstance;
	createInfo.hwnd = W_MainWindow::hWnd;

	vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);
}