#pragma once

#include <optional>
#include <vector>

#include "Concepts.hh"
#include "Core.hh"
#include "Error.hh"

namespace ignis {

template <typename T>
class Pool : public NonCopyable, public NonMovable {
   public:
    explicit Pool(u64 size) : m_pool(size) {}

    template <typename... Args>
        requires std::constructible_from<T, Args...>
    Result<u64> create(Args&&... args) {
        if (auto slot = findFreeSlot(); slot.has_value()) [[likely]] {
            m_pool[slot.value()].emplace(std::forward<Args>(args)...);
            m_size++;
            return slot.value();
        }
        return Error::unexpected(
            Error::Code::poolFull, "No free slots available in pool"
        );
    }

    template <typename Constructor>
        requires Callable<Constructor, T(u32)>
    Result<u64> create(Constructor&& constructor) {
        if (auto slot = findFreeSlot(); slot.has_value()) [[likely]] {
            m_pool[slot.value()].emplace(constructor(slot.value()));
            m_size++;
            return slot.value();
        }
        return Error::unexpected(
            Error::Code::poolFull, "No free slots available in pool"
        );
    }

    void destroy(u64 id) {
        if (id < m_pool.size()) {
            m_pool[id].reset();
            m_size--;
        }
    }

    T* get(u64 id) {
        if (id < m_pool.size() && m_pool[id].has_value()) {
            return &m_pool[id].value();
        }
        return nullptr;
    }

    const T* get(u64 id) const {
        if (id < m_pool.size() && m_pool[id].has_value()) {
            return &m_pool[id].value();
        }
        return nullptr;
    }

    u64 size() const { return m_pool.size(); }

   private:
    std::optional<u64> findFreeSlot() const {
        for (u64 i = 0; i < m_pool.size(); ++i) {
            if (not m_pool[i].has_value()) {
                return i;
            }
        }
        return std::nullopt;
    }

    u64 m_size{0u};
    std::vector<std::optional<T>> m_pool;
};

}  // namespace ignis
