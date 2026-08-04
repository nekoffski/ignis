#pragma once

#include "Core.hh"

namespace ignis {

template <typename T, T Type>
struct HandleBase {
    u32 id : 24;
    u32 generation : 8 {0u};

    static T type() { return Type; }

    bool operator==(const HandleBase&) const = default;
    bool operator<(const HandleBase& o) const {
        return (id << 8u | generation) < (o.id << 8u | o.generation);
    }
};

}  // namespace ignis
