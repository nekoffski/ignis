#pragma once

#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"

namespace ember {

struct Window : public NonCopyable, public NonMovable {
    virtual ~Window() = default;
};

}  // namespace ember
