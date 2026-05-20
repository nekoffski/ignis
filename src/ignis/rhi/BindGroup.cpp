#include "BindGroup.hh"

#include "ResourceManager.hh"

namespace ignis::rhi {

BindGroupProxy::BindGroupProxy(
    ResourceManager& resources, BindGroupHandle handle
)
    : m_resources(resources), m_handle(handle) {}

Opt<Error> BindGroupProxy::set(
    const DescriptorLocation& location, TextureHandle texture
) {}

Opt<Error> BindGroupProxy::set(
    const DescriptorLocation& location, BufferHandle buffer
) {}

Opt<Error> BindGroupProxy::set(
    const PushConstantLocation& location, const void* data
) {}

}  // namespace ignis::rhi
