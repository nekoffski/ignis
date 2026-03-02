#pragma once

#include <memory>

#include "Window.hh"
#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"

namespace ember {

class RHI : public NonCopyable, public NonMovable {
   public:
    virtual ~RHI() = default;

    static std::unique_ptr<RHI> create(Window* window = nullptr);

   private:
};

}  // namespace ember
