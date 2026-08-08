#pragma once

#include "ignis/core/Core.hh"
#include "ignis/core/Handle.hh"

namespace ignis::rhi {

enum class ResourceType : u8 {
    buffer,
    texture,
    renderPass,
    shader,
    pipeline,
    bindGroup
};

template <ResourceType T>
struct Handle : HandleBase<ResourceType, T> {
    using HandleBase<ResourceType, T>::HandleBase;
};

template <typename T, ResourceType RT>
struct ResourceWrapper {
    T resource;
    Handle<RT> handle;
};

using BufferHandle = Handle<ResourceType::buffer>;
using RenderPassHandle = Handle<ResourceType::renderPass>;
using ShaderHandle = Handle<ResourceType::shader>;
using TextureHandle = Handle<ResourceType::texture>;
using PipelineHandle = Handle<ResourceType::pipeline>;
using BindGroupHandle = Handle<ResourceType::bindGroup>;

}  // namespace ignis::rhi

template <ignis::rhi::ResourceType T>
struct std::hash<ignis::rhi::Handle<T>> {
    size_t operator()(const ignis::rhi::Handle<T>& h) const noexcept {
        const auto key = static_cast<ignis::u64>(h.id) << 32u | h.generation;
        return std::hash<ignis::u64>{}(key);
    }
};
