#pragma once

#include <memory>

#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"

namespace ember {

class RHI : public NonCopyable, public NonMovable {
   public:
    static std::unique_ptr<RHI> create();

   private:
};

}  // namespace ember
