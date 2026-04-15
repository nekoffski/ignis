#pragma once

#include "VK.hh"
#include "VKBuffer.hh"
#include "VKCommandBuffer.hh"
#include "VKDeviceInfo.hh"
#include "VKQueue.hh"
#include "VKTexture.hh"
#include "ignis/core/Pool.hh"
#include "ignis/core/Scoped.hh"
#include "ignis/rhi/Device.hh"

namespace ignis {

class VKDevice : public Device {
    using BufferPool =
        Pool<DeviceResourceWrapper<VKBuffer, DeviceResourceType::buffer>>;
    using TexturePool =
        Pool<DeviceResourceWrapper<VKTexture, DeviceResourceType::texture>>;

   public:
    explicit VKDevice(const Config& config, Window* window);
    ~VKDevice() override = default;

    bool headless() const override;

    VkInstance instance() const;
    VkPhysicalDevice physicalDevice() const;
    VkDevice device() const;
    Allocator allocator() const;
    VKDeviceInfo deviceInfo() const;
    const VKCommandPools& commandPools() const;

    Result<DeviceWorkloadReceipt> submit(
        const DeviceWorkload& workload) override;
    OError wait(DeviceWorkloadReceipt receipt) override;

    Result<DeviceBufferHandle> createBuffer(
        const DeviceBufferDescription& desc) override;
    void destroyBuffer(DeviceBufferHandle handle) override;

    Result<DeviceTextureHandle> createTexture(
        const DeviceTextureDefinition& definition) override;
    void destroyTexture(DeviceTextureHandle handle) override;

    OError transfer(HostToBufferTransfer copy) override;

    OError transfer(BufferToTextureTransfer copy) override;
    OError transfer(BufferToTextureTransfer copy,
                    DeviceWorkload& workload) override;

    OError transfer(TextureToBufferTransfer copy,
                    DeviceWorkload& workload) override;
    OError transfer(TextureToBufferTransfer copy) override;

    OError transfer(BufferToHostTransfer copy) override;

    Opt<i32> findMemoryIndex(u32 typeFilter,
                             DeviceMemoryProperty memoryProperty);

    bool supportsFormat(DeviceTextureFormat format, DeviceTextureTiling tiling,
                        DeviceTextureUsage usage);

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

    BufferPool m_bufferPool;
    TexturePool m_texturePool;
};

}  // namespace ignis
