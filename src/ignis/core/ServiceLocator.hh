#pragma once

#include "Concepts.hh"
#include "Log.hh"

namespace ignis {

template <typename T>
class ServiceLocator : public StaticClass {
   public:
    static T& get() {
        log::expect(s_instance, "Service {} is not registered",
                    typeid(T).name());
        return *s_instance;
    }

    static T* find() { return s_instance; }

    static void set(T* instance) {
        log::expect(instance, "Cannot set service {} to nullptr",
                    typeid(T).name());
        s_instance = instance;
    }

   private:
    inline static T* s_instance{nullptr};
};

}  // namespace ignis
