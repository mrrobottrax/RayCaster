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
#pragma warning(pop)

#endif // VULKAN

// Windows
#ifdef WINDOWS

#include "Windows.h"

#endif // WINDOWS