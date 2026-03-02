#pragma once

#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"

namespace ember {

class Renderer;

class CommandList : public NonCopyable, public NonMovable {
    friend class Renderer;

    struct Guard {};

   public:
    explicit CommandList(Guard);

   private:
};

}  // namespace ember
