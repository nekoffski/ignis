#include "VKCommandDispatcher.hh"

#include "VKCommands.hh"
#include "VKDevice.hh"
#include "ignis/core/Functional.hh"

namespace ignis::rhi {

#define CHECK_QUEUE(cmd, expectedQueue)                                   \
    if (cmd.targetQueue() != expectedQueue) {                             \
        log::panic("Command {} is not meant for the target queue", #cmd); \
    }

VKCommandManifest& VKCommandManifest::add(RenderPassHandle handle) {
    m_renderPasses.insert(handle);
    return *this;
}
VKCommandManifest& VKCommandManifest::add(TextureHandle handle) {
    m_textures.insert(handle);
    return *this;
}

VKCommandManifest& VKCommandManifest::add(BufferHandle handle) {
    m_buffers.insert(handle);
    return *this;
}

const std::unordered_set<RenderPassHandle>& VKCommandManifest::renderPasses(
) const {
    return m_renderPasses;
}

const std::unordered_set<TextureHandle>& VKCommandManifest::textures() const {
    return m_textures;
}

const std::unordered_set<BufferHandle>& VKCommandManifest::buffers() const {
    return m_buffers;
}

VKCommandDispatcher::VKCommandDispatcher(
    VKDevice& device, VkCommandBuffer cmdBuffer, Queue targetQueue
)
    : m_device(device),
      m_cmdBuffer(cmdBuffer),
      m_targetQueue(targetQueue),
      m_context(cmdBuffer) {}

Opt<Error> VKCommandDispatcher::dispatch(const Command& command) {
    return dispatch(std::span<const Command>{&command, 1});
}

Opt<Error> VKCommandDispatcher::dispatch(std::span<const Command> commands) {
    if (auto err = preprocessCommands(commands); err) return err;
    return recordCommands(commands);
}

Opt<Error> VKCommandDispatcher::recordCommands(std::span<const Command> commands
) {
    for (const auto& command : commands) {
        if (auto err = recordCommand(command); err) {
            log::error("Failed to record command: {}", err->message());
            return err;
        }
    }
    log::debug("Successfully recorded {} commands", commands.size());
    return Error::empty();
}

Opt<Error> VKCommandDispatcher::preprocessCommands(
    std::span<const Command> commands
) {
    for (const auto& command : commands) {
        if (auto err = preprocessCommand(command); err) {
            log::error("Failed to preprocess command: {}", err->message());
            return err;
        }
    }

    if (auto err = m_context.consume(m_manifest, m_device); err) {
        log::error("Failed to preprocess commands: {}", err->message());
        return err;
    }

    log::debug("Successfully preprocessed {} commands", commands.size());
    return Error::empty();
}

VKCommandContext::VKCommandContext(VkCommandBuffer cmdBuffer)
    : m_cmdBuffer(cmdBuffer) {}

VKRenderPass& VKCommandContext::resource(RenderPassHandle handle) const {
    auto it = m_renderPasses.find(handle);
    log::expect(
        it != m_renderPasses.end(), "Render pass with handle {} not found",
        static_cast<u32>(handle.id)
    );
    return *it->second;
}

VKTexture& VKCommandContext::resource(TextureHandle handle) const {
    auto it = m_textures.find(handle);
    log::expect(
        it != m_textures.end(), "Texture with handle {} not found",
        static_cast<u32>(handle.id)
    );
    return *it->second;
}

VKBuffer& VKCommandContext::resource(BufferHandle handle) const {
    auto it = m_buffers.find(handle);
    log::expect(
        it != m_buffers.end(), "Buffer with handle {} not found",
        static_cast<u32>(handle.id)
    );
    return *it->second;
}

VkCommandBuffer VKCommandContext::cmdBuffer() const { return m_cmdBuffer; }

Opt<Error> VKCommandContext::consume(
    const VKCommandManifest& manifest, VKDevice& device
) {
    for (const auto& renderPassHandle : manifest.renderPasses()) {
        auto renderPass = device.findRenderPass(renderPassHandle);
        if (not renderPass) {
            return Error{
                Error::Code::resourceMissing,
                fmt::format(
                    "Render pass with handle {} not found",
                    static_cast<u32>(renderPassHandle.id)
                )
            };
        }
        m_renderPasses[renderPassHandle] = renderPass;
    }

    for (const auto& textureHandle : manifest.textures()) {
        auto texture = device.findTexture(textureHandle);
        if (not texture) {
            return Error{
                Error::Code::resourceMissing,
                fmt::format(
                    "Texture with handle {} not found",
                    static_cast<u32>(textureHandle.id)
                )
            };
        }
        m_textures[textureHandle] = texture;
    }

    for (const auto& bufferHandle : manifest.buffers()) {
        auto buffer = device.findBuffer(bufferHandle);
        if (not buffer) {
            return Error{
                Error::Code::resourceMissing,
                fmt::format(
                    "Buffer with handle {} not found",
                    static_cast<u32>(bufferHandle.id)
                )
            };
        }
        m_buffers[bufferHandle] = buffer;
    }

    return Error::empty();
}

Opt<Error> VKCommandDispatcher::recordCommand(const Command& command) {
    Overloader visitor{
        [&](const CmdUploadBufferToTexture& cmd) -> Opt<Error> {
            return rhi::recordCommand(m_context, cmd);
        },
        [&](const CmdDownloadTextureToBuffer& cmd) -> Opt<Error> {
            return rhi::recordCommand(m_context, cmd);
        },
        [&](const CmdBeginRenderPass& cmd) -> Opt<Error> {
            return rhi::recordCommand(m_context, cmd);
        },
        [&](const CmdEndRenderPass& cmd) -> Opt<Error> {
            return rhi::recordCommand(m_context, cmd);
        },
    };
    return std::visit(std::move(visitor), command);
}

Opt<Error> VKCommandDispatcher::preprocessCommand(const Command& command) {
    Overloader visitor{
        [&](const CmdUploadBufferToTexture& cmd) -> Opt<Error> {
            CHECK_QUEUE(cmd, m_targetQueue);
            return rhi::preprocessCommand(m_manifest, cmd);
        },
        [&](const CmdDownloadTextureToBuffer& cmd) -> Opt<Error> {
            CHECK_QUEUE(cmd, m_targetQueue);
            return rhi::preprocessCommand(m_manifest, cmd);
        },
        [&](const CmdBeginRenderPass& cmd) -> Opt<Error> {
            CHECK_QUEUE(cmd, m_targetQueue);
            return rhi::preprocessCommand(m_manifest, cmd);
        },
        [&](const CmdEndRenderPass& cmd) -> Opt<Error> {
            CHECK_QUEUE(cmd, m_targetQueue);
            return rhi::preprocessCommand(m_manifest, cmd);
        },
    };
    return std::visit(std::move(visitor), command);
}

}  // namespace ignis::rhi
