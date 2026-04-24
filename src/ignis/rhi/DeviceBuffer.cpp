#include "DeviceBuffer.hh"

#include "Device.hh"

namespace ignis {

DeviceBufferDescription DeviceBufferDescription::staging(u64 size) {
    DeviceBufferDescription d{};

    d.size = size;
    d.usage = DeviceBufferUsage::transferSrc | DeviceBufferUsage::transferDst;
    d.memoryProperty =
        DeviceMemoryProperty::hostVisible | DeviceMemoryProperty::hostCoherent;
    d.bindOnCreation = true;

    return d;
}

Opt<Error> DeviceBufferProxy::write(const void* data, const Range& range) {
    if (auto* impl = m_device.proxy(m_handle); impl) {
        impl->write(data, range);
        return Error::empty();
    }
    return Error{Error::Code::invalidArgument,
                 "Failed to get buffer proxy: invalid buffer handle"};
}

Opt<Error> DeviceBufferProxy::read(void* data, const Range& range) {
    if (auto* impl = m_device.proxy(m_handle); impl) {
        impl->read(data, range);
        return Error::empty();
    }
    return Error{Error::Code::invalidArgument,
                 "Failed to get buffer proxy: invalid buffer handle"};
}

DeviceBufferProxy::DeviceBufferProxy(Device& device,
                                     DeviceBufferHandle handleDevice)
    : m_device(device), m_handle(handleDevice) {}

}  // namespace ignis
