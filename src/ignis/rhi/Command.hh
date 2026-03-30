#pragma once

#include <variant>

#include "Queue.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

enum class CommandType : u8 {
    draw,
    dispatch,
    copyBuffer,
    copyImage,
    clearColorImage,
    clearDepthStencilImage,
    pipelineBarrier,
};

template <CommandType Type, Queue TargetQueue>
struct CommandBase {
    static CommandType type() { return Type; }
    static Queue targetQueue() { return TargetQueue; }
};

struct DrawCommand : public CommandBase<CommandType::draw, Queue::graphics> {
    u32 vertexCount{0};
    u32 instanceCount{1};
    u32 firstVertex{0};
    u32 firstInstance{0};
};

using Command = std::variant<DrawCommand>;

template <typename T>
concept CommandConcept = requires {
    { T::type() } -> std::same_as<CommandType>;
    { T::targetQueue() } -> std::same_as<Queue>;
    { T{} } -> std::convertible_to<Command>;
};

}  // namespace ignis
