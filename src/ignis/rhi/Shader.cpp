#include "Shader.hh"

#include "ignis/core/Log.hh"

namespace ignis::rhi {

std::string shaderStageToString(ShaderStage stage) {
    switch (stage) {
        case ShaderStage::none:
            return "none";
        case ShaderStage::vertex:
            return "vertex";
        case ShaderStage::fragment:
            return "fragment";
        case ShaderStage::compute:
            return "compute";
        case ShaderStage::geometry:
            return "geometry";
        default:
            return "unknown";
    }
}

ShaderStage shaderStageFromString(const std::string& str) {
    if (str == "none") {
        return ShaderStage::none;
    } else if (str == "vertex") {
        return ShaderStage::vertex;
    } else if (str == "fragment") {
        return ShaderStage::fragment;
    } else if (str == "compute") {
        return ShaderStage::compute;
    } else if (str == "geometry") {
        return ShaderStage::geometry;
    } else {
        log::error("Unknown shader stage type '{}'", str);
        return ShaderStage::none;
    }
}

}  // namespace ignis::rhi
