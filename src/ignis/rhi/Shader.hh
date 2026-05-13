#pragma once

#include "ResourceHandle.hh"
#include "ignis/core/Enum.hh"

namespace ignis::rhi {

enum class ShaderStage {
    none,
    vertex,
    fragment,
    compute,
    geometry,
};
IGNIS_BIT_ENUM(ShaderStage);

struct ShaderDescription {};

using ShaderHandle = ResourceHandle<ResourceType::shader>;

std::string shaderStageToString(ShaderStage stage);
ShaderStage shaderStageFromString(const std::string& str);

}  // namespace ignis::rhi
