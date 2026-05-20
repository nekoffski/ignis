#pragma once

#include "ResourceHandle.hh"
#include "Shader.hh"

namespace ignis::rhi {

using BindGroupHandle = ResourceHandle<ResourceType::bindGroup>;

struct BindGroupDescription {
    ShaderHandle shader;
};

class ResourceManager;

class BindGroupProxy : public NonCopyable, public NonMovable {
   public:
    struct PushConstantLocation {
        u32 offset;
        u32 size;
    };

    struct DescriptorLocation {
        u32 set;
        u32 binding;
    };

    struct Impl : public virtual NonCopyable {
        virtual ~Impl() = default;

        virtual Opt<Error> set(
            const DescriptorLocation& location, TextureHandle texture
        ) = 0;
        virtual Opt<Error> set(
            const DescriptorLocation& location, BufferHandle buffer
        ) = 0;
        virtual Opt<Error> set(
            const PushConstantLocation& location, const void* data
        ) = 0;
    };

    explicit BindGroupProxy(ResourceManager& resources, BindGroupHandle handle);

    Opt<Error> set(const DescriptorLocation& location, TextureHandle texture);
    Opt<Error> set(const DescriptorLocation& location, BufferHandle buffer);
    Opt<Error> set(const PushConstantLocation& location, const void* data);

   private:
    ResourceManager& m_resources;
    BindGroupHandle m_handle;
};

}  // namespace ignis::rhi
