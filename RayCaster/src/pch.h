#pragma once

#define _CRTDBG_MAP_ALLOC
#define _CRT_SECURE_NO_WARNINGS

// STD
#include <stdlib.h>
#include <crtdbg.h>

#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime> 
#include <thread>
#include <random>

// Vulkan
#ifdef WINDOWS

#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX // fixes bugs

#endif // WINDOWS

#pragma warning(push)
#pragma warning( disable : 28251 )
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

// Windows
#ifdef WINDOWS

#include <_platform/windows/W_Include.h>

#endif // WINDOWS

#undef _CRT_SECURE_NO_WARNINGS