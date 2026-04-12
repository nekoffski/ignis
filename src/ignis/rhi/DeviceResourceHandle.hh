#pragma once

#include "ignis/core/Core.hh"

namespace ignis {

enum class DeviceResourceType : u8 { buffer, texture };

template <DeviceResourceType T>
struct DeviceResourceHandle {
    u32 id : 24;
    u32 generation : 8 {0u};

    static DeviceResourceType type() { return T; }
};

template <typename T, DeviceResourceType ResourceType>
struct DeviceResourceWrapper {
    T resource;
    DeviceResourceHandle<ResourceType> handle;
};

}  // namespace ignis
