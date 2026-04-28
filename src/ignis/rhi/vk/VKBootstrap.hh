#pragma once

#include "VK.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Queue.hh"
#include "ignis/rhi/Window.hh"

namespace ignis::rhi {

class VKBootstrap : public NonCopyable, public NonMovable {
   public:
    struct DeviceRequirements {
        Queue queues;
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
    VKQueueSet queues() const;

    VKCommandPools& commandPools();

   private:
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPools();
    void fetchQueues();

    const Config& m_cfg;
    Window* m_window;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VKDeviceInfo m_deviceInfo;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    Allocator m_allocator{nullptr};
    VKCommandPools m_commandPools;
    VKQueueSet m_queues;
};

}  // namespace ignis::rhi
