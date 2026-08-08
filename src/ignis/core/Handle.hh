#pragma once

#include "Core.hh"

namespace ignis {

struct HandleKey {
    u32 id{std::numeric_limits<u32>::max()};
    u32 generation{0u};

    constexpr HandleKey() = default;
    constexpr HandleKey(u32 id, u32 generation)
        : id(id), generation(generation) {}

    bool operator==(const HandleKey&) const = default;
    bool operator<(const HandleKey& other) const {
        return id < other.id ||
               (id == other.id && generation < other.generation);
    }
};

template <typename T, T Type>
struct HandleBase : HandleKey {
    using HandleKey::HandleKey;

    static T type() { return Type; }

    bool operator==(const HandleBase&) const = default;
    bool operator<(const HandleBase& other) const {
        return HandleKey::operator<(other);
    }
};

}  // namespace ignis
