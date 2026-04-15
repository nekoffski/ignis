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
            return slot.value();
        }
        return Error::unexpected(Error::Code::poolFull,
                                 "No free slots available in pool");
    }

    template <typename Constructor>
        requires Callable<Constructor, T(u32)>
    Result<u64> create(Constructor&& constructor) {
        if (auto slot = findFreeSlot(); slot.has_value()) [[likely]] {
            m_pool[slot.value()].emplace(constructor(slot.value()));
            return slot.value();
        }
        return Error::unexpected(Error::Code::poolFull,
                                 "No free slots available in pool");
    }

    void destroy(u64 id) {
        if (id < m_pool.size()) m_pool[id].reset();
    }

    T* get(u64 id) {
        if (id < m_pool.size() && m_pool[id].has_value())
            return &m_pool[id].value();
        return nullptr;
    }

    const T* get(u64 id) const {
        if (id < m_pool.size() && m_pool[id].has_value())
            return &m_pool[id].value();
        return nullptr;
    }

   private:
    std::optional<u64> findFreeSlot() const {
        for (u64 i = 0; i < m_pool.size(); ++i)
            if (not m_pool[i].has_value()) return i;
        return std::nullopt;
    }

    std::vector<std::optional<T>> m_pool;
};

}  // namespace ignis
