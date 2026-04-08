#pragma once

#include "VK.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceQueue.hh"
#include "ignis/rhi/Window.hh"

namespace ignis {

class VKBootstrap : public NonCopyable, public NonMovable {
   public:
    struct DeviceRequirements {
        DeviceQueue queues;
        bool supportSurface;
        bool isDiscrete;
        std::vector<const char*> extensions;

        std::optional<VKDeviceInfo> fulfills(
            const VkPhysicalDevice& device) const;
    };

    explicit VKBootstrap(const Config& config, Window* window);

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VkDebugUtilsMessengerEXT debugMessenger() const;
    const VKDeviceInfo& deviceInfo() const;
    VkCommandPool graphicsCommandPool() const;
    VKQueueSet queues() const;

   private:
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createGraphicsCommandPool();
    void fetchQueues();

    const Config& m_cfg;
    Window* m_window;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VKDeviceInfo m_deviceInfo;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    Allocator m_allocator{nullptr};
    VkCommandPool m_graphicsCommandPool{VK_NULL_HANDLE};
    VKQueueSet m_queues;
};

}  // namespace ignis
