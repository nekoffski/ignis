// #pragma once

// #include <vulkan/vulkan.h>

// #include <optional>
// #include <unordered_map>
// #include <vector>

// #include "ignis/core/Concepts.hh"
// #include "ignis/core/Core.hh"
// #include "ignis/rhi/Queue.hh"
// #include "ignis/rhi/Window.hh"

// namespace ignis {

// class VK : public NonCopyable, public NonMovable {
//    public:
//     struct DeviceInfo {
//         VkPhysicalDeviceProperties coreProperties;
//         VkPhysicalDeviceMemoryProperties memoryProperties;
//         VkPhysicalDeviceFeatures features;
//         std::unordered_map<Queue::Type, u32> queueIndices;
//         VkSurfaceCapabilitiesKHR surfaceCapabilities;
//         std::vector<VkSurfaceFormatKHR> surfaceFormats;
//         std::vector<VkPresentModeKHR> presentModes;
//         VkFormat depthFormat;
//         u8 depthChannelCount;
//         VkSurfaceFormatKHR surfaceFormat;
//         VkPresentModeKHR presentMode;
//         bool supportsDeviceLocalHostVisibleMemory;
//     };

//     struct DeviceRequirements {
//         Queue::Type queues;
//         bool supportSurface;
//         bool isDiscrete;
//         std::vector<const char*> extensions;

//         std::optional<DeviceInfo> fulfills(
//             const VkPhysicalDevice& device) const;
//     };

//     explicit VK(Window* window);
//     ~VK();

//     bool headless() const;
//     const DeviceInfo& deviceInfo() const;

//    private:
//     void createInstance();
//     void createDebugMessenger();
//     void pickPhysicalDevice();
//     void createLogicalDevice();

//     std::pair<std::unordered_map<Queue::Type, u32>, Queue::Type>
//     assignQueues(
//         const VK::DeviceRequirements& req) const;

//     const VkAllocationCallbacks* allocator() const;

//     Window* m_window{nullptr};

//     VkInstance m_instance{VK_NULL_HANDLE};
//     VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
//     DeviceInfo m_deviceInfo;
//     VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
//     VkDevice m_device{VK_NULL_HANDLE};
// };

// #define VK_ASSERT(x)                                                          \
//     do {                                                                      \
//         log::trace("Vulkan call: {}", #x);                                    \
//         auto res = (x);                                                       \
//         if (res != VK_SUCCESS) {                                              \
//             log::panic("Vulkan error: {} at {}:{}: {}", fmt::underlying(res), \
//                        __FILE__, __LINE__, #x);                               \
//         }                                                                     \
//     } while (0)

// #define VK_TRACE(x)                        \
//     do {                                   \
//         log::trace("Vulkan call: {}", #x); \
//         (x);                               \
//     } while (0)

// }  // namespace ignis
