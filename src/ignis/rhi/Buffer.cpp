#include "Buffer.hh"

#include "Device.hh"

namespace ignis::rhi {

BufferDescription BufferDescription::staging(u64 size) {
    BufferDescription d{};

    d.size = size;
    d.usage = BufferUsage::transferSrc | BufferUsage::transferDst;
    d.memoryProperty =
        MemoryProperty::hostVisible | MemoryProperty::hostCoherent;
    d.bindOnCreation = true;

    return d;
}

Opt<Error> BufferProxy::write(const void* data, const Range& range) {
    if (auto* impl = m_device.proxy(m_handle); impl) {
        impl->write(data, range);
        return Error::empty();
    }
    return Error{Error::Code::invalidArgument,
                 "Failed to get buffer proxy: invalid buffer handle"};
}

Opt<Error> BufferProxy::read(void* data, const Range& range) {
    if (auto* impl = m_device.proxy(m_handle); impl) {
        impl->read(data, range);
        return Error::empty();
    }
    return Error{Error::Code::invalidArgument,
                 "Failed to get buffer proxy: invalid buffer handle"};
}

BufferProxy::BufferProxy(Device& device, BufferHandle handle)
    : m_device(device), m_handle(handle) {}

}  // namespace ignis::rhi
