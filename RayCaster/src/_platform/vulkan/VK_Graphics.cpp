#include "pch.h"
#include "VK_Graphics.h"
#include <_wrappers/console/ConsoleWrapper.h>

void VK_Init()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    Print("%u extensions supported", extensionCount);
    // std::cout << extensionCount << " extensions supported\n";
}
