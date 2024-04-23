#pragma once

void CreateDevice();

VkPhysicalDevice PickPhysicalDevice();
int RatePhysicalDeviceSuitability(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);
bool IsDeviceSuitable(VkPhysicalDevice device, const VkPhysicalDeviceProperties& properties);

bool GetDeviceExtensionSupport(VkPhysicalDevice device);