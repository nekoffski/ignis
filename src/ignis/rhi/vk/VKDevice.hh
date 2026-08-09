#pragma once

#include <unordered_map>
#include <vector>

#include "VK.hh"
#include "VKCommandBuffer.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "VKResourceManager.hh"
#include "VKSync.hh"
#include "ignis/core/Pool.hh"
#include "ignis/core/Scoped.hh"
#include "ignis/rhi/Device.hh"

namespace ignis::rhi {

class VKDevice : public Device {
   public:
    explicit VKDevice(const Config& config, Window* window);
    ~VKDevice() override;

    bool headless() const override;
    Format depthFormat() const override;
    const DeviceCapabilities& capabilities() const override;

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
    struct PendingWorkload {
        HandleKey key;
        TimelinePoint completion;
    };

    TimelinePoint reserveTimelinePoint(Queue queue);
    Opt<Error> validateTimelinePoint(TimelinePoint point) const;
    void collectCompletedWorkloads();

    const Config& m_cfg;
    Window* m_window;

    Scoped<VkInstance> m_instance;
    Scoped<VkDebugUtilsMessengerEXT> m_debugMessenger;
    VkPhysicalDevice m_physicalDevice;
    Scoped<VkDevice> m_device;
    Allocator m_allocator{nullptr};
    VKDeviceInfo m_deviceInfo;
    DeviceCapabilities m_capabilities;
    Scoped<VKCommandPools> m_commandPools;
    VKQueueSet m_queues;
    std::unordered_map<Queue, std::unique_ptr<VKTimelineSemaphore>> m_timelines;
    std::unordered_map<Queue, u64> m_nextTimelineValues;
    std::unordered_map<Queue, u64> m_lastSubmittedTimelineValues;

    Pool<VKWorkload> m_pendingWorkloads;
    std::vector<PendingWorkload> m_pendingWorkloadKeys;
    VKResourceManager m_resourceManager;
};

}  // namespace ignis::rhi
