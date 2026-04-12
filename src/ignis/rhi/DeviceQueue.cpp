#include "DeviceQueue.hh"

namespace ignis {

Str toString(DeviceQueue q) {
    switch (q) {
        case DeviceQueue::none:
            return "none";
        case DeviceQueue::graphics:
            return "graphics";
        case DeviceQueue::compute:
            return "compute";
        case DeviceQueue::transfer:
            return "transfer";
        case DeviceQueue::present:
            return "present";
        default:
            return "unknown";
    }
}

}  // namespace ignis
