#pragma once

#include <span>
#include <unordered_map>
#include <unordered_set>

#include "VK.hh"
#include "VKBuffer.hh"
#include "VKRenderPass.hh"
#include "VKTexture.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Error.hh"
#include "ignis/rhi/Command.hh"

namespace ignis::rhi {

class VKDevice;
class VKCommandDispatcher;
class VKCommandContext;

class VKCommandManifest : public NonCopyable, public NonMovable {
    friend class VKCommandDispatcher;

   public:
    VKCommandManifest() = default;

    VKCommandManifest& add(RenderPassHandle handle);
    VKCommandManifest& add(TextureHandle handle);
    VKCommandManifest& add(BufferHandle handle);

    const std::unordered_set<RenderPassHandle>& renderPasses() const;
    const std::unordered_set<TextureHandle>& textures() const;
    const std::unordered_set<BufferHandle>& buffers() const;

   private:
    std::unordered_set<RenderPassHandle> m_renderPasses;
    std::unordered_set<TextureHandle> m_textures;
    std::unordered_set<BufferHandle> m_buffers;
};

class VKCommandContext : public NonCopyable, public NonMovable {
    friend class VKCommandDispatcher;

   public:
    explicit VKCommandContext(VkCommandBuffer cmdBuffer);

    VKRenderPass& resource(RenderPassHandle handle) const;
    VKTexture& resource(TextureHandle handle) const;
    VKBuffer& resource(BufferHandle handle) const;

    VkCommandBuffer cmdBuffer() const;

   private:
    VkCommandBuffer m_cmdBuffer{VK_NULL_HANDLE};

    std::unordered_map<RenderPassHandle, VKRenderPass*> m_renderPasses;
    std::unordered_map<TextureHandle, VKTexture*> m_textures;
    std::unordered_map<BufferHandle, VKBuffer*> m_buffers;

    Opt<Error> consume(const VKCommandManifest& manifest, VKDevice& device);
};

class VKCommandDispatcher : public NonCopyable, public NonMovable {
   public:
    explicit VKCommandDispatcher(
        VKDevice& device, VkCommandBuffer cmdBuffer, Queue targetQueue
    );

    Opt<Error> dispatch(const Command& command);
    Opt<Error> dispatch(std::span<const Command> commands);

   private:
    Opt<Error> recordCommands(std::span<const Command> commands);
    Opt<Error> recordCommand(const Command& command);

    Opt<Error> preprocessCommands(std::span<const Command> commands);
    void preprocessCommand(const Command& command);

    VKDevice& m_device;
    VkCommandBuffer m_cmdBuffer;
    Queue m_targetQueue;

    VKCommandManifest m_manifest;
    VKCommandContext m_context;
};

}  // namespace ignis::rhi
