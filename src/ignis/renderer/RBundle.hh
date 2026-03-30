#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

using RResourceTag = u64;

class RBundle : public NonCopyable, public NonMovable {
   public:
    explicit RBundle() = default;

   private:
};

}  // namespace ignis
