#pragma once

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

// std
#include "stdlib.h"
#include "crtdbg.h"
#include "assert.h"
#include "stdio.h"
#include "iostream"
#include "string"
#include "chrono"
#include "ctime" 
#include "thread"
#include "random"
#include "map"
#include "optional"
#include "set"

// ----------------------------

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

#include "_platform/windows/w_include.h"

#endif // WINDOWS

// ----------------------------

#include "core.h"

#undef _CRT_SECURE_NO_WARNINGS