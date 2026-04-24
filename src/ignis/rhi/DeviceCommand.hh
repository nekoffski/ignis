#pragma once

#include <variant>

#include "DeviceQueue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceBuffer.hh"
#include "ignis/rhi/DeviceTexture.hh"

namespace ignis {

enum class DeviceCommandType : u8 {
    bufferToTextureUpload,
    textureToBufferDownload
};

template <DeviceCommandType Type, DeviceQueue TargetQueue>
struct CommandBase {
    static DeviceCommandType type() { return Type; }
    static DeviceQueue targetQueue() { return TargetQueue; }
};

struct CmdUploadBufferToTexture
    : public CommandBase<DeviceCommandType::bufferToTextureUpload,
                         DeviceQueue::transfer> {
    DeviceBufferHandle from;
    DeviceTextureHandle to;
};

struct CmdDownloadTextureToBuffer
    : public CommandBase<DeviceCommandType::textureToBufferDownload,
                         DeviceQueue::transfer> {
    DeviceTextureHandle from;
    DeviceBufferHandle to;
};

using DeviceCommand =
    std::variant<CmdUploadBufferToTexture, CmdDownloadTextureToBuffer>;

template <typename T>
concept CommandConcept = requires {
    { T::type() } -> std::same_as<DeviceCommandType>;
    { T::targetQueue() } -> std::same_as<DeviceQueue>;
    { T{} } -> std::convertible_to<DeviceCommand>;
};

}  // namespace ignis
