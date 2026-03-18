#pragma once

#include <nlohmann/json.hpp>

namespace ignis {

template <typename T>
concept JsonSerializable = requires(T a) {
    { a.toJson() } -> std::convertible_to<nlohmann::json>;
};

template <typename T>
concept JsonDeserializable = requires(const nlohmann::json& j) {
    { T::fromJson(j) } -> std::convertible_to<T>;
};

}  // namespace ignis
