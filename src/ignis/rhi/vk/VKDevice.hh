#pragma once

#include "VK.hh"
#include "VKCommandBuffer.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "VKResourceManager.hh"
#include "ignis/core/Pool.hh"
#include "ignis/core/Scoped.hh"
#include "ignis/rhi/Device.hh"

namespace ignis::rhi {

class VKDevice : public Device {
   public:
    explicit VKDevice(const Config& config, Window* window);
    ~VKDevice() override = default;

    bool headless() const override;
    Format depthFormat() const override;

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VKDeviceInfo deviceInfo() const;
    const VKCommandPools& commandPools() const;

    Result<WorkloadReceipt> submit(const Workload& workload) override;
    Opt<Error> wait(WorkloadReceipt receipt) override;

    Opt<i32> findMemoryIndex(u32 typeFilter, MemoryProperty memoryProperty);

    bool supportsFormat(Format format, Tiling tiling, TextureUsage usage);

    VkQueue queue(Queue type) const;
    u32 queueIndex(Queue type) const;

    VKResourceManager& resources() override;

   private:
    const Config& m_cfg;
    Window* m_window;

    Scoped<VkInstance> m_instance;
    Scoped<VkDebugUtilsMessengerEXT> m_debugMessenger;
    VkPhysicalDevice m_physicalDevice;
    Scoped<VkDevice> m_device;
    Allocator m_allocator{nullptr};
    VKDeviceInfo m_deviceInfo;
    Scoped<VKCommandPools> m_commandPools;
    VKQueueSet m_queues;

    Pool<VKWorkload> m_pendingWorkloads;
    VKResourceManager m_resourceManager;
};

}  // namespace ignis::rhi
