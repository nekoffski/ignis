#pragma once

#include "ignis/core/Core.hh"

namespace ignis {

enum class ResourceType : u8 { buffer, texture };

template <ResourceType T>
struct ResourceHandle {
    u32 id : 24;
    u32 generation : 8;

    ResourceType type() const { return T; }
};

}  // namespace ignis
 