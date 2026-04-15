#include "DeviceBuffer.hh"

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

}  // namespace ignis
