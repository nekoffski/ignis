#pragma once

#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"

namespace ignis {

enum class DeviceQueue : u8 {
    none = 0,
    graphics = 1 << 0,
    compute = 1 << 1,
    transfer = 1 << 2,
    present = 1 << 3
};

IGNIS_BIT_ENUM(DeviceQueue);

}  // namespace ignis
