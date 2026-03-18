// #pragma once

// #include "ignis/core/Concepts.hh"
// #include "ignis/core/Core.hh"

// namespace ignis {

// struct Queue : NonCopyable, NonMovable {
//     enum class Type {
//         none = 0,
//         graphics,
//         compute,
//         transfer,
//         present,
//     };

//     virtual ~Queue() = default;
//     virtual Type type() const = 0;
// };

// }  // namespace ignis

// constexpr void enableBitOperations(ignis::Queue::Type);
