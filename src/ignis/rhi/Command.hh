#pragma once

#include <variant>

#include "Queue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/BindGroup.hh"
#include "ignis/rhi/Buffer.hh"
#include "ignis/rhi/Pipeline.hh"
#include "ignis/rhi/RenderPass.hh"
#include "ignis/rhi/Texture.hh"

namespace ignis::rhi {

enum class CommandType : u8 {
    bufferToTextureUpload,
    textureToBufferDownload,
    beginRenderPass,
    endRenderPass,
    bindPipeline,
    bindBindGroup,
    setViewport,
    setScissor,
    draw,
    drawIndexed,
    bindVertexBuffer,
    bindIndexBuffer,
};

template <CommandType Type, Queue TargetQueue>
struct CommandBase {
    static CommandType type() { return Type; }
    static Queue targetQueue() { return TargetQueue; }
};

struct CmdUploadBufferToTexture
    : public CommandBase<CommandType::bufferToTextureUpload, Queue::transfer> {
    BufferHandle from;
    TextureHandle to;
};

struct CmdDownloadTextureToBuffer
    : public CommandBase<
          CommandType::textureToBufferDownload, Queue::transfer> {
    TextureHandle from;
    BufferHandle to;
};

struct CmdBeginRenderPass
    : public CommandBase<CommandType::beginRenderPass, Queue::graphics> {
    RenderPassHandle renderPass;
    std::vector<TextureHandle> attachments;
    Rect<f32> renderArea;
    Vec4 clearColor;
};

struct CmdEndRenderPass
    : public CommandBase<CommandType::endRenderPass, Queue::graphics> {
    RenderPassHandle renderPass;
};

struct CmdBindPipeline
    : public CommandBase<CommandType::bindPipeline, Queue::graphics> {
    PipelineHandle pipeline;
};

struct CmdSetViewport
    : public CommandBase<CommandType::setViewport, Queue::graphics> {
    Rect<f32> area;
    f32 minDepth{0.f};
    f32 maxDepth{1.f};
};

struct CmdSetScissor
    : public CommandBase<CommandType::setScissor, Queue::graphics> {
    Rect<f32> area;
};

struct CmdDraw : public CommandBase<CommandType::draw, Queue::graphics> {
    u32 vertexCount;
    u32 instanceCount{1};
    u32 firstVertex{0};
    u32 firstInstance{0};
};

struct CmdDrawIndexed
    : public CommandBase<CommandType::drawIndexed, Queue::graphics> {
    u32 indexCount;
    u32 instanceCount{1};
    u32 firstIndex{0};
    i32 vertexOffset{0};
    u32 firstInstance{0};
};

struct CmdBindBindGroup
    : public CommandBase<CommandType::bindBindGroup, Queue::graphics> {
    BindGroupHandle bindGroup;
};

struct CmdBindVertexBuffer
    : public CommandBase<CommandType::bindVertexBuffer, Queue::graphics> {
    BufferHandle buffer;
    u64 offset{0};
};

struct CmdBindIndexBuffer
    : public CommandBase<CommandType::bindIndexBuffer, Queue::graphics> {
    BufferHandle buffer;
    u64 offset{0};
};

using Command = std::variant<
    CmdUploadBufferToTexture, CmdDownloadTextureToBuffer, CmdBeginRenderPass,
    CmdEndRenderPass, CmdBindPipeline, CmdBindBindGroup, CmdSetViewport,
    CmdSetScissor, CmdDraw, CmdDrawIndexed, CmdBindVertexBuffer,
    CmdBindIndexBuffer>;

template <typename T>
concept CommandConcept = requires {
    { T::type() } -> std::same_as<CommandType>;
    { T::targetQueue() } -> std::same_as<Queue>;
    { T{} } -> std::convertible_to<Command>;
};

}  // namespace ignis::rhi
