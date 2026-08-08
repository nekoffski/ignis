#pragma once

#include <optional>
#include <vector>

#include "Concepts.hh"
#include "Core.hh"
#include "Error.hh"
#include "Handle.hh"

namespace ignis {

template <typename T, std::unsigned_integral Generation = u32>
class Pool : public NonCopyable, public NonMovable {
    static_assert(sizeof(Generation) <= sizeof(u32));

   public:
    explicit Pool(u64 size) : m_pool(size) {}

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    Result<HandleKey> create(Args&&... args) {
        if (auto slot = findFreeSlot(); slot.has_value()) [[likely]] {
            auto& entry = m_pool[slot.value()];
            entry.value.emplace(std::forward<Args>(args)...);
            m_size++;
            return HandleKey{
                static_cast<u32>(*slot), static_cast<u32>(entry.generation)
            };
        }
        return Error::unexpected(
            Error::Code::poolFull, "No free slots available in pool"
        );
    }

    template <typename Constructor>
        requires Callable<Constructor, T(HandleKey)>
    Result<HandleKey> create(Constructor&& constructor) {
        if (auto slot = findFreeSlot(); slot.has_value()) [[likely]] {
            auto& entry = m_pool[slot.value()];
            auto key = HandleKey{
                static_cast<u32>(*slot), static_cast<u32>(entry.generation)
            };
            entry.value.emplace(constructor(key));
            m_size++;
            return key;
        }
        return Error::unexpected(
            Error::Code::poolFull, "No free slots available in pool"
        );
    }

    bool destroy(HandleKey key) {
        if (key.id >= m_pool.size()) {
            return false;
        }

        auto& entry = m_pool[key.id];
        if (entry.generation != key.generation || not entry.value.has_value()) {
            return false;
        }

        entry.value.reset();
        m_size--;

        if (entry.generation == std::numeric_limits<Generation>::max()) {
            entry.retired = true;
        } else {
            entry.generation++;
        }
        return true;
    }

    T* get(HandleKey key) {
        if (key.id < m_pool.size()) {
            auto& entry = m_pool[key.id];
            if (entry.generation == key.generation && entry.value.has_value()) {
                return &entry.value.value();
            }
        }
        return nullptr;
    }

    const T* get(HandleKey key) const {
        if (key.id < m_pool.size()) {
            const auto& entry = m_pool[key.id];
            if (entry.generation == key.generation && entry.value.has_value()) {
                return &entry.value.value();
            }
        }
        return nullptr;
    }

    u64 liveCount() const { return m_size; }
    u64 capacity() const { return m_pool.size(); }

   private:
    struct Slot {
        std::optional<T> value;
        Generation generation{0u};
        bool retired{false};
    };

    std::optional<u64> findFreeSlot() const {
        for (u64 i = 0; i < m_pool.size(); ++i) {
            if (not m_pool[i].retired && not m_pool[i].value.has_value()) {
                return i;
            }
        }
        return std::nullopt;
    }

    u64 m_size{0u};
    std::vector<Slot> m_pool;
};

}  // namespace ignis
