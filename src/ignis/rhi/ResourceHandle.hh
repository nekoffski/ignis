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
struct Handle : HandleBase<ResourceType, T> {};

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
        return std::hash<ignis::u32>{}(h.id << 8u | h.generation);
    }
};
