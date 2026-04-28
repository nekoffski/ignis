#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

struct Window : public NonCopyable, public NonMovable {
    virtual ~Window() = default;
};

}  // namespace ignis::rhi
