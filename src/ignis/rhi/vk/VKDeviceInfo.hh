#pragma once

#include <unordered_map>
#include <vector>

#include "VK.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceCapabilities.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis::rhi {

struct VKDeviceInfo {
    VkPhysicalDeviceProperties coreProperties;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceVulkan12Features vulkan12Features;
    VkPhysicalDeviceVulkan13Features vulkan13Features;
    VkPhysicalDeviceDriverProperties driverProperties;
    DeviceCapabilities capabilities;
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

struct VKCommandPools {
    VkCommandPool graphics{VK_NULL_HANDLE};
    VkCommandPool transfer{VK_NULL_HANDLE};
};

}  // namespace ignis::rhi
