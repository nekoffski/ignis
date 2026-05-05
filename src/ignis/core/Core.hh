#pragma once

#include <fmt/core.h>

#include <cstdint>
#include <limits>
#include <optional>

namespace ignis {

using u64 = uint64_t;
using u32 = uint32_t;
using u16 = uint16_t;
using u8 = uint8_t;

template <typename T>
using Opt = std::optional<T>;

using Nanoseconds = u64;

using i64 = int64_t;
using i32 = int32_t;
using i16 = int16_t;
using i8 = int8_t;

using f32 = float;
using f64 = double;
using Str = std::string;

template <typename T>
T max() {
    return std::numeric_limits<T>::max();
}

template <typename T>
T min() {
    return std::numeric_limits<T>::min();
}

template <typename T>
T getAlignedValue(T value, T granularity) {
    return (value + granularity - 1) & ~(granularity - 1);
}

struct Range {
    u64 offset;
    u64 size;

    static Range aligned(u64 offset, u64 size, u64 granularity);
};

template <typename T>
class Wrapper {
   public:
    explicit Wrapper(const T& value) : m_value(value) {}

    const T& get() const { return m_value; }

    bool operator==(const Wrapper& other) const {
        return m_value == other.m_value;
    }

    bool operator!=(const Wrapper& other) const {
        return m_value != other.m_value;
    }

   private:
    T m_value;
};

template <typename T>
struct Rect {
    T x;
    T y;
    T w;
    T h;
};

using Name = Wrapper<Str>;

template <typename T>
class Tag : public Wrapper<T> {
   public:
    using Wrapper<T>::Wrapper;
};

template <>
class Tag<Str> : public Wrapper<Str> {
   public:
    using Wrapper<Str>::Wrapper;

    static Tag<Str> fromUuid();
};

}  // namespace ignis

template <typename T>
struct std::hash<ignis::Wrapper<T>> {
    std::size_t operator()(const ignis::Wrapper<T>& v) const noexcept {
        return std::hash<T>{}(v.get());
    }
};

template <typename T>
struct std::hash<ignis::Tag<T>> {
    std::size_t operator()(const ignis::Tag<T>& v) const noexcept {
        return std::hash<T>{}(v.get());
    }
};
