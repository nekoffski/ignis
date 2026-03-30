#pragma once

#include "RPass.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

using RGraphId = u64;

class RGraph : public NonCopyable, public NonMovable {};

}  // namespace ignis
