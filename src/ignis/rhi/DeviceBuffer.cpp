#include "DeviceBuffer.hh"

namespace ignis {

DeviceBufferDescription DeviceBufferDescription::staging(u64 size) {
    DeviceBufferDescription d{};

    d.size = size;
    d.usage = DeviceBufferUsage::transferSrcBit;
    d.memoryProperty = DeviceMemoryProperty::hostVisibleBit |
                       DeviceMemoryProperty::hostCoherentBit;
    d.bindOnCreation = true;

    return d;
}

}  // namespace ignis
