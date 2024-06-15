#pragma once

// Vulkan
#ifdef VULKAN

#ifdef WINDOWS // Vulkan on Windows stuff

#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX // fixes bugs

#endif // WINDOWS

#pragma warning(push)
#pragma warning( disable : 28251 )
#include "vulkan/vulkan.hpp"
#include "vulkan/vk_enum_string_helper.h"
#pragma warning(pop)

#endif // VULKAN

// Windows
#ifdef WINDOWS

#include "Windows.h"
#include "_platform/windows/w_error.h"
#include <winDNS.h>

#endif // WINDOWS