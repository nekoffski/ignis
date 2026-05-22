#pragma once

#include <span>
#include <unordered_map>
#include <unordered_set>

#include "VK.hh"
#include "VKBindGroup.hh"
#include "VKBuffer.hh"
#include "VKPipeline.hh"
#include "VKRenderPass.hh"
#include "VKTexture.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Error.hh"
#include "ignis/rhi/Command.hh"

namespace ignis::rhi {

class VKResourceManager;
class VKCommandDispatcher;
class VKCommandContext;

class VKCommandManifest : public NonCopyable, public NonMovable {
    friend class VKCommandDispatcher;

   public:
    VKCommandManifest() = default;

    VKCommandManifest& add(RenderPassHandle handle);
    VKCommandManifest& add(TextureHandle handle);
    VKCommandManifest& add(BufferHandle handle);
    VKCommandManifest& add(PipelineHandle handle);
    VKCommandManifest& add(BindGroupHandle handle);

    const std::unordered_set<RenderPassHandle>& renderPasses() const;
    const std::unordered_set<TextureHandle>& textures() const;
    const std::unordered_set<BufferHandle>& buffers() const;
    const std::unordered_set<PipelineHandle>& pipelines() const;
    const std::unordered_set<BindGroupHandle>& bindGroups() const;

   private:
    std::unordered_set<RenderPassHandle> m_renderPasses;
    std::unordered_set<TextureHandle> m_textures;
    std::unordered_set<BufferHandle> m_buffers;
    std::unordered_set<PipelineHandle> m_pipelines;
    std::unordered_set<BindGroupHandle> m_bindGroups;
};

class VKCommandContext : public NonCopyable, public NonMovable {
    friend class VKCommandDispatcher;

   public:
    explicit VKCommandContext(VkCommandBuffer cmdBuffer);

    VKRenderPass& resource(RenderPassHandle handle) const;
    VKTexture& resource(TextureHandle handle) const;
    VKBuffer& resource(BufferHandle handle) const;
    VKPipeline& resource(PipelineHandle handle) const;
    VKBindGroup& resource(BindGroupHandle handle) const;

    VkCommandBuffer cmdBuffer() const;

   private:
    VkCommandBuffer m_cmdBuffer{VK_NULL_HANDLE};

    std::unordered_map<RenderPassHandle, VKRenderPass*> m_renderPasses;
    std::unordered_map<TextureHandle, VKTexture*> m_textures;
    std::unordered_map<BufferHandle, VKBuffer*> m_buffers;
    std::unordered_map<PipelineHandle, VKPipeline*> m_pipelines;
    std::unordered_map<BindGroupHandle, VKBindGroup*> m_bindGroups;

    Opt<Error> consume(
        const VKCommandManifest& manifest, VKResourceManager& resourceManager
    );
};

class VKCommandDispatcher : public NonCopyable, public NonMovable {
   public:
    explicit VKCommandDispatcher(
        VKResourceManager& resourceManager, VkCommandBuffer cmdBuffer,
        Queue targetQueue
    );

    Opt<Error> dispatch(const Command& command);
    Opt<Error> dispatch(std::span<const Command> commands);

   private:
    Opt<Error> recordCommands(std::span<const Command> commands);
    Opt<Error> recordCommand(const Command& command);

    Opt<Error> preprocessCommands(std::span<const Command> commands);
    Opt<Error> preprocessCommand(const Command& command);

    VKResourceManager& m_resourceManager;
    VkCommandBuffer m_cmdBuffer;
    Queue m_targetQueue;

    VKCommandManifest m_manifest;
    VKCommandContext m_context;
};

}  // namespace ignis::rhi
