#pragma once

#include "ignis/core/Core.hh"

namespace ignis::rhi {

enum class ResourceType : u8 { buffer, texture };

template <ResourceType T>
struct ResourceHandle {
    u32 id : 24;
    u32 generation : 8 {0u};

    static ResourceType type() { return T; }
};

template <typename T, ResourceType RT>
struct ResourceWrapper {
    T resource;
    ResourceHandle<RT> handle;
};

}  // namespace ignis::rhi
