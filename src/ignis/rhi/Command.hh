#pragma once

#include <variant>

#include "Queue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Buffer.hh"
#include "ignis/rhi/Texture.hh"

namespace ignis::rhi {

enum class CommandType : u8 { bufferToTextureUpload, textureToBufferDownload };

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
    : public CommandBase<CommandType::textureToBufferDownload,
                         Queue::transfer> {
    TextureHandle from;
    BufferHandle to;
};

using Command =
    std::variant<CmdUploadBufferToTexture, CmdDownloadTextureToBuffer>;

template <typename T>
concept CommandConcept = requires {
    { T::type() } -> std::same_as<CommandType>;
    { T::targetQueue() } -> std::same_as<Queue>;
    { T{} } -> std::convertible_to<Command>;
};

}  // namespace ignis::rhi
