#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"

namespace ignis {

enum class ResourceAccess { colorAttachment, depthAttachment, read };
enum class RenderPassType { graphics, compute, copy };

}  // namespace ignis
