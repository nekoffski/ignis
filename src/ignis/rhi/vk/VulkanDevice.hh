#pragma once

#include "Vulkan.hh"
#include "VulkanDeviceInfo.hh"
#include "VulkanQueue.hh"
#include "VulkanResourceRegistry.hh"
#include "ignis/rhi/Device.hh"

namespace ignis {

class VulkanDevice : public Device {
   public:
    explicit VulkanDevice(const Config& config, Window* window);
    ~VulkanDevice() override;

    bool headless() const override;

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VulkanDeviceInfo deviceInfo() const;

    WorkloadReceipt submit(const Workload& workload) override;
    void wait(WorkloadReceipt receipt) override;

   private:
    const Config& m_cfg;
    Window* m_window;

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
    Allocator m_allocator{nullptr};
    VulkanDeviceInfo m_deviceInfo;
    VkCommandPool m_graphicsCommandPool{VK_NULL_HANDLE};
    VulkanQueueSet m_queues;
};

}  // namespace ignis
