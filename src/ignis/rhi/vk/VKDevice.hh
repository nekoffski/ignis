#pragma once

#include "VK.hh"
#include "VKBuffer.hh"
#include "VKCommandBuffer.hh"
#include "VKDeviceInfo.hh"
#include "VKPipeline.hh"
#include "VKQueue.hh"
#include "VKRenderPass.hh"
#include "VKShader.hh"
#include "VKTexture.hh"
#include "ignis/core/Pool.hh"
#include "ignis/core/Scoped.hh"
#include "ignis/rhi/Device.hh"

namespace ignis::rhi {

class VKDevice : public Device {
    using BufferPool = Pool<ResourceWrapper<VKBuffer, ResourceType::buffer>>;
    using TexturePool = Pool<ResourceWrapper<VKTexture, ResourceType::texture>>;
    using RenderPassPool =
        Pool<ResourceWrapper<VKRenderPass, ResourceType::renderPass>>;
    using ShaderPool = Pool<ResourceWrapper<VKShader, ResourceType::shader>>;
    using PipelinePool =
        Pool<ResourceWrapper<VKPipeline, ResourceType::pipeline>>;

    friend class BufferProxy;

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

    Result<BufferHandle> createBuffer(const BufferDescription& desc) override;
    void destroyBuffer(BufferHandle handle) override;

    Result<TextureHandle> createTexture(const TextureDescription& definition
    ) override;
    void destroyTexture(TextureHandle handle) override;

    Result<RenderPassHandle> createRenderPass(const RenderPassDescription& desc
    ) override;
    void destroyRenderPass(RenderPassHandle handle) override;

    Result<ShaderHandle> createShader(const ShaderDescription& shaderDescription
    ) override;
    void destroyShader(ShaderHandle handle) override;

    Result<PipelineHandle> createPipeline(
        const PipelineDescription& pipelineDescription
    ) override;
    void destroyPipeline(PipelineHandle handle) override;

    Opt<i32> findMemoryIndex(u32 typeFilter, MemoryProperty memoryProperty);

    bool supportsFormat(Format format, Tiling tiling, TextureUsage usage);

    VkQueue queue(Queue type) const;
    u32 queueIndex(Queue type) const;

    VKTexture* findTexture(TextureHandle handle);
    VKBuffer* findBuffer(BufferHandle handle);
    VKRenderPass* findRenderPass(RenderPassHandle handle);
    VKShader* findShader(ShaderHandle handle);
    VKPipeline* findPipeline(PipelineHandle handle);

   private:
    BufferProxy::Impl* proxy(BufferHandle handle) override;

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
    RenderPassPool m_renderPassPool;
    ShaderPool m_shaderPool;
    PipelinePool m_pipelinePool;
};

}  // namespace ignis::rhi
