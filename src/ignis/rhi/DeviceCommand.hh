#pragma once

#include <variant>

#include "DeviceQueue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

enum class DeviceCommandType : u8 {
    draw,
    dispatch,
    copyBuffer,
    copyImage,
    clearColorImage,
    clearDepthStencilImage,
    pipelineBarrier,
};

template <DeviceCommandType Type, DeviceQueue TargetQueue>
struct CommandBase {
    static DeviceCommandType type() { return Type; }
    static DeviceQueue targetQueue() { return TargetQueue; }
};

struct DrawCommand
    : public CommandBase<DeviceCommandType::draw, DeviceQueue::graphics> {
    u32 vertexCount{0};
    u32 instanceCount{1};
    u32 firstVertex{0};
    u32 firstInstance{0};
};

using DeviceCommand = std::variant<DrawCommand>;

template <typename T>
concept CommandConcept = requires {
    { T::type() } -> std::same_as<DeviceCommandType>;
    { T::targetQueue() } -> std::same_as<DeviceQueue>;
    { T{} } -> std::convertible_to<DeviceCommand>;
};

}  // namespace ignis
