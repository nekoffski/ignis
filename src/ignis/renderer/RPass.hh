#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

enum class RAcess { colorAttachment, depthAttachment, read };
enum class RPassType { graphics, compute, copy };

}  // namespace ignis
