#pragma once

#include <functional>

#include "ignis/core/Concepts.hh"

namespace ignis {

template <typename T>
class Scoped : public NonCopyable {
   public:
    Scoped() = default;

    template <typename Callable>
        requires std::invocable<Callable, T&>
    Scoped(T resource, Callable&& destructor)
        : m_resource(resource),
          m_destructor(std::forward<Callable>(destructor)),
          m_empty(false) {}

    ~Scoped() {
        if (not m_empty) {
            m_destructor(m_resource);
        }
    }

    Scoped(Scoped&& oth)
        : m_resource(std::move(oth.m_resource)),
          m_destructor(std::move(oth.m_destructor)),
          m_empty(oth.m_empty) {
        oth.m_empty = true;
    }

    Scoped& operator=(Scoped&& oth) {
        if (this != &oth) {
            if (not m_empty) {
                m_destructor(m_resource);
            }
            m_resource = std::move(oth.m_resource);
            m_destructor = std::move(oth.m_destructor);
            m_empty = oth.m_empty;
            oth.m_empty = true;
        }
        return *this;
    }

    T& operator*() { return m_resource; }
    const T& operator*() const { return m_resource; }

    T& get() { return m_resource; }
    const T& get() const { return m_resource; }

    bool empty() const { return m_empty; }

   private:
    T m_resource;
    std::function<void(T&)> m_destructor;
    bool m_empty{true};
};

}  // namespace ignis
