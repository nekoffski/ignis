#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

using ResourceTag = u64;

class RenderBundle : public NonCopyable, public NonMovable {
   public:
    explicit RenderBundle() = default;

   private:
};

}  // namespace ignis
