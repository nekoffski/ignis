#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis::render {

using ResourceTag = Tag<u64>;

class FrameBundle : public NonCopyable, public NonMovable {
   public:
    explicit FrameBundle() = default;

   private:
};

}  // namespace ignis::render
