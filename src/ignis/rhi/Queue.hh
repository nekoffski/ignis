#pragma once

#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"

namespace ignis::rhi {

enum class Queue : u8 {
    none = 0,
    graphics = 1 << 0,
    compute = 1 << 1,
    transfer = 1 << 2,
    present = 1 << 3
};

Str toString(Queue q);

IGNIS_BIT_ENUM(Queue);

}  // namespace ignis::rhi
