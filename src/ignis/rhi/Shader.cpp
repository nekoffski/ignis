#include "Shader.hh"

#include <fmt/format.h>

#include "Texture.hh"
#include "ignis/core/Log.hh"

namespace ignis::rhi {

std::string shaderStageToString(ShaderStageType stage) {
    if (stage == ShaderStageType::none) return "none";

    std::string result;
    auto append = [&](ShaderStageType flag, const char* name) {
        if (checkFlag(stage, flag)) {
            if (not result.empty()) result += '|';
            result += name;
        }
    };
    append(ShaderStageType::vertex, "vertex");
    append(ShaderStageType::fragment, "fragment");
    append(ShaderStageType::compute, "compute");
    append(ShaderStageType::geometry, "geometry");
    return result.empty() ? "unknown" : result;
}

ShaderStageType shaderStageTypeFromString(const std::string& str) {
    if (str == "none") {
        return ShaderStageType::none;
    } else if (str == "vertex") {
        return ShaderStageType::vertex;
    } else if (str == "fragment") {
        return ShaderStageType::fragment;
    } else if (str == "compute") {
        return ShaderStageType::compute;
    } else if (str == "geometry") {
        return ShaderStageType::geometry;
    } else {
        log::error("Unknown shader stage type '{}'", str);
        return ShaderStageType::none;
    }
}

Str toString(DescriptorType type) {
    switch (type) {
        case DescriptorType::uniformBuffer:
            return "uniformBuffer";
        case DescriptorType::storageBuffer:
            return "storageBuffer";
        case DescriptorType::sampledTexture:
            return "sampledTexture";
        case DescriptorType::storageTexture:
            return "storageTexture";
        case DescriptorType::sampler:
            return "sampler";
        default:
            return fmt::format("Unknown({})", fmt::underlying(type));
    }
}

Str toString(const DescriptorBinding& b) {
    return fmt::format(
        "binding(set={}, binding={}, type={}, count={}, stages={})", b.set,
        b.binding, toString(b.type), b.count, shaderStageToString(b.stageFlags)
    );
}

Str toString(const PushConstantRange& pc) {
    return fmt::format(
        "pushConstant(offset={}, size={}, stages={})", pc.offset, pc.size,
        shaderStageToString(pc.stageFlags)
    );
}

Str toString(const VertexAttribute& attr) {
    return fmt::format(
        "vertexAttr(location={}, format={})", attr.location,
        toString(attr.format)
    );
}

Str toString(const ShaderDescription& desc) {
    Str result = fmt::format(
        "ShaderDescription(stages={})\n", shaderStageToString(desc.stagesMask)
    );
    for (const auto& b : desc.bindings)
        result += fmt::format("  {}\n", toString(b));
    for (const auto& pc : desc.pushConstants)
        result += fmt::format("  {}\n", toString(pc));
    for (const auto& attr : desc.vertexAttributes)
        result += fmt::format("  {}\n", toString(attr));
    return result;
}

}  // namespace ignis::rhi
