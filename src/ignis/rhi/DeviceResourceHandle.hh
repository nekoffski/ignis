#pragma once

#include "ignis/core/Core.hh"

namespace ignis {

enum class DeviceResourceType : u8 { buffer, texture };

template <DeviceResourceType T>
struct DeviceResourceHandle {
    u32 id : 24;
    u32 generation : 8;

    static DeviceResourceType type() { return T; }
};

}  // namespace ignis
