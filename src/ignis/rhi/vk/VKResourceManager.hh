#pragma once

#include "ignis/core/Config.hh"
#include "ignis/core/Pool.hh"
#include "ignis/rhi/ResourceManager.hh"

#include "VKBuffer.hh"
#include "VKTexture.hh"
#include "VKRenderPass.hh"
#include "VKShader.hh"
#include "VKPipeline.hh"

namespace ignis::rhi {

class VKDevice;

class VKResourceManager : public ResourceManager {
    using BufferPool = Pool<ResourceWrapper<VKBuffer, ResourceType::buffer>>;
    using TexturePool = Pool<ResourceWrapper<VKTexture, ResourceType::texture>>;
    using RenderPassPool =
        Pool<ResourceWrapper<VKRenderPass, ResourceType::renderPass>>;
    using ShaderPool = Pool<ResourceWrapper<VKShader, ResourceType::shader>>;
    using PipelinePool =
        Pool<ResourceWrapper<VKPipeline, ResourceType::pipeline>>;

   public:
    explicit VKResourceManager(VKDevice& device, const Config& config);

    Result<BufferHandle> create(const BufferDescription& bufferDescription
    ) override;
    void destroy(BufferHandle handle) override;

    Result<TextureHandle> create(const TextureDescription& textureDescription
    ) override;
    void destroy(TextureHandle handle) override;

    Result<RenderPassHandle> create(
        const RenderPassDescription& renderPassDescription
    ) override;
    void destroy(RenderPassHandle handle) override;

    Result<ShaderHandle> create(const ShaderDescription& shaderDescription
    ) override;
    void destroy(ShaderHandle handle) override;

    Result<PipelineHandle> create(const PipelineDescription& pipelineDescription
    ) override;
    void destroy(PipelineHandle handle) override;

    VKTexture* find(TextureHandle handle);
    VKBuffer* find(BufferHandle handle);
    VKRenderPass* find(RenderPassHandle handle);
    VKShader* find(ShaderHandle handle);
    VKPipeline* find(PipelineHandle handle);

   private:
    BufferProxy::Impl* proxy(BufferHandle handle) override;

    VKDevice& m_device;

    BufferPool m_bufferPool;
    TexturePool m_texturePool;
    RenderPassPool m_renderPassPool;
    ShaderPool m_shaderPool;
    PipelinePool m_pipelinePool;
};

}  // namespace ignis::rhi
