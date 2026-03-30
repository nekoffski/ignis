#pragma once

#include <unordered_map>
#include <vector>

#include "VK.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis {

struct VKDeviceInfo {
    VkPhysicalDeviceProperties coreProperties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkPhysicalDeviceFeatures features;
    std::unordered_map<Queue, u32> queueIndices;
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
    VkFormat depthFormat;
    u8 depthChannelCount;
    VkSurfaceFormatKHR surfaceFormat;
    VkPresentModeKHR presentMode;
    bool supportsDeviceLocalHostVisibleMemory;
};

}  // namespace ignis
