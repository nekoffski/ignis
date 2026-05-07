#pragma once

#include "Concepts.hh"
#include "Core.hh"
#include "Log.hh"
#include "RTTI.hh"

namespace ignis {

template <typename T>
class Singleton : public virtual NonMovable, public virtual NonCopyable {
    inline static std::string className = getTypeName<T>();

   public:
    [[nodiscard]] static T& get() {
        static T* instance = new T{};  // let it leak
        return *instance;
    }

    static void noop() {}
};

template <typename T>
class UniqueInstance : public virtual NonMovable, public virtual NonCopyable {
    inline static std::string className = getTypeName<T>();

   public:
    explicit UniqueInstance() {
        log::expect(
            not s_instanceExists, "Instance of {} already exists", className
        );
        s_instanceExists = true;
    }

   private:
    inline static bool s_instanceExists = false;
};

}  // namespace ignis
