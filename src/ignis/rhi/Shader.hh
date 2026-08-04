#pragma once

#include <vector>

#include "Buffer.hh"
#include "ResourceHandle.hh"
#include "Texture.hh"
#include "ignis/core/Enum.hh"

namespace ignis::rhi {

enum class ShaderStageType {
    none = 0x0,
    vertex = 0x1,
    fragment = 0x2,
    compute = 0x4,
    geometry = 0x8,
};
IGNIS_BIT_ENUM(ShaderStageType);

enum class DescriptorType {
    uniformBuffer,
    storageBuffer,
    sampledTexture,
    storageTexture,
    sampler,
};

struct DescriptorBinding {
    u32 set;
    u32 binding;
    DescriptorType type;
    u32 count;
    ShaderStageType stageFlags;
};

struct PushConstantRange {
    u32 offset;
    u32 size;
    ShaderStageType stageFlags;
};

struct VertexAttribute {
    u32 location;
    Format format;
};

struct ShaderStage {
    ShaderStageType stage;
    std::string entrypoint;
    std::vector<u32> spirv;
};

struct ShaderDescription {
    ShaderStageType stagesMask;
    std::vector<ShaderStage> stages;
    std::vector<DescriptorBinding> bindings;
    std::vector<PushConstantRange> pushConstants;
    std::vector<VertexAttribute> vertexAttributes;
};

std::string shaderStageToString(ShaderStageType stage);
ShaderStageType shaderStageTypeFromString(const std::string& str);

Str toString(DescriptorType type);
Str toString(const DescriptorBinding& binding);
Str toString(const PushConstantRange& pc);
Str toString(const VertexAttribute& attr);
Str toString(const ShaderDescription& desc);

}  // namespace ignis::rhi
