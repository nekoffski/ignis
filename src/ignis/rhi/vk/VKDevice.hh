#pragma once

#include "VK.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "VKResourceRegistry.hh"
#include "ignis/rhi/Device.hh"

namespace ignis {

class VKDevice : public Device {
   public:
    explicit VKDevice(const Config& config, Window* window);
    ~VKDevice() override;

    bool headless() const override;

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VKDeviceInfo deviceInfo() const;

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
    VKDeviceInfo m_deviceInfo;
    VkCommandPool m_graphicsCommandPool{VK_NULL_HANDLE};
    VKQueueSet m_queues;
};

}  // namespace ignis
