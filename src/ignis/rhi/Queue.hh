#pragma once

#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"

namespace ignis {

enum class Queue : u8 {
    none = 0,
    graphics = 1 << 0,
    compute = 1 << 1,
    transfer = 1 << 2,
    present = 1 << 3
};

BIT_ENUM(Queue);

}  // namespace ignis
