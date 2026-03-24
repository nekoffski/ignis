#pragma once

#include "Vulkan.hh"
#include "VulkanDeviceInfo.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Queue.hh"
#include "ignis/rhi/Window.hh"

namespace ignis {

class VulkanBootstrap : public NonCopyable, public NonMovable {
   public:
    struct DeviceRequirements {
        Queue queues;
        bool supportSurface;
        bool isDiscrete;
        std::vector<const char*> extensions;

        std::optional<VulkanDeviceInfo> fulfills(
            const VkPhysicalDevice& device) const;
    };

    explicit VulkanBootstrap(const Config& config, Window* window);

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VkDebugUtilsMessengerEXT debugMessenger() const;
    const VulkanDeviceInfo& deviceInfo() const;

   private:
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();

    const Config& m_cfg;
    Window* m_window;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VulkanDeviceInfo m_deviceInfo;
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    Allocator m_allocator{nullptr};
};

}  // namespace ignis
