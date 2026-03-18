#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

class Renderer;

class RenderGraph : public NonCopyable, public NonMovable {
    friend class Renderer;

    struct Guard {};

   public:
    explicit RenderGraph(Guard) {}

   private:
};

}  // namespace ignis
