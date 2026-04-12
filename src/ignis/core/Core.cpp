#include "Core.hh"

#include "Random.hh"

namespace ignis {

Tag<Str> Tag<Str>::fromUuid() { return Tag<Str>{RandomEngine::get().uuid()}; }

Range Range::aligned(u64 offset, u64 size, u64 granularity) {
    return Range{
        .offset = getAlignedValue(offset, granularity),
        .size = getAlignedValue(size, granularity),
    };
}

}  // namespace ignis
