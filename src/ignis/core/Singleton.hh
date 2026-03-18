#pragma once

#include "Concepts.hh"
#include "Core.hh"
#include "RTTI.hh"

namespace ignis {

template <typename T>
class Singleton : public virtual NonMovable, public virtual NonCopyable {
    inline static std::string className = getTypeName<T>();

   public:
    static T& get() {
        static T* instance = new T{};  // let it leak
        return *instance;
    }
};

}  // namespace ignis
