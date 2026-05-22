#include "BindGroup.hh"

#include "ResourceManager.hh"

namespace ignis::rhi {

BindGroupProxy::BindGroupProxy(
    ResourceManager& resources, BindGroupHandle handle
)
    : m_resources(resources), m_handle(handle) {}

Opt<Error> BindGroupProxy::set(
    const DescriptorLocation& location, TextureHandle texture
) {
    if (auto* impl = m_resources.proxy(m_handle); impl)
        return impl->set(location, texture);
    return Error{Error::Code::resourceMissing, "BindGroup not found"};
}

Opt<Error> BindGroupProxy::set(
    const DescriptorLocation& location, BufferHandle buffer
) {
    if (auto* impl = m_resources.proxy(m_handle); impl)
        return impl->set(location, buffer);
    return Error{Error::Code::resourceMissing, "BindGroup not found"};
}

Opt<Error> BindGroupProxy::set(
    const PushConstantLocation& location, const void* data
) {
    if (auto* impl = m_resources.proxy(m_handle); impl)
        return impl->set(location, data);
    return Error{Error::Code::resourceMissing, "BindGroup not found"};
}

}  // namespace ignis::rhi
