#pragma once

#include "ignis/core/Core.hh"

namespace ignis::rhi {

enum class ResourceType : u8 { buffer, texture, renderPass };

template <ResourceType T>
struct ResourceHandle {
    u32 id : 24;
    u32 generation : 8 {0u};

    static ResourceType type() { return T; }

    bool operator==(const ResourceHandle&) const = default;
    bool operator<(const ResourceHandle& o) const {
        return (id << 8u | generation) < (o.id << 8u | o.generation);
    }
};

template <typename T, ResourceType RT>
struct ResourceWrapper {
    T resource;
    ResourceHandle<RT> handle;
};

}  // namespace ignis::rhi

template <ignis::rhi::ResourceType T>
struct std::hash<ignis::rhi::ResourceHandle<T>> {
    size_t operator()(const ignis::rhi::ResourceHandle<T>& h) const noexcept {
        return std::hash<ignis::u32>{}(h.id << 8u | h.generation);
    }
};
