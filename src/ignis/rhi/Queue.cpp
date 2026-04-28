#include "Queue.hh"

namespace ignis::rhi {

Str toString(Queue q) {
    switch (q) {
        case Queue::none:
            return "none";
        case Queue::graphics:
            return "graphics";
        case Queue::compute:
            return "compute";
        case Queue::transfer:
            return "transfer";
        case Queue::present:
            return "present";
        default:
            return "unknown";
    }
}

}  // namespace ignis::rhi
