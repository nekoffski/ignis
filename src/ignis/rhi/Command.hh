#pragma once

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

struct CommandBase {};

}  // namespace ignis
