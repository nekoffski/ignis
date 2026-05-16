#pragma once

#include "RenderPass.hh"
#include "ResourceHandle.hh"
#include "Shader.hh"
#include "Texture.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

using PipelineHandle = ResourceHandle<ResourceType::pipeline>;

enum class PolygonMode : u8 { fill, line, point };
enum class CullMode : u8 { none, front, back };
enum class FrontFace : u8 { clockwise, counterClockwise };

struct RasterizerDescription {
    PolygonMode polygonMode{PolygonMode::fill};
    CullMode cullMode{CullMode::back};
    FrontFace frontFace{FrontFace::counterClockwise};
    f32 lineWidth{1.0f};
};

enum class CompareOp : u8 {
    never,
    less,
    equal,
    lessOrEqual,
    greater,
    notEqual,
    greaterOrEqual,
    always
};

struct DepthDescription {
    bool testEnable{true};
    bool writeEnable{true};
    CompareOp compareOp{CompareOp::less};
};

enum class BlendFactor : u8 {
    zero,
    one,
    srcColor,
    oneMinusSrcColor,
    dstColor,
    oneMinusDstColor,
    srcAlpha,
    oneMinusSrcAlpha,
    dstAlpha,
    oneMinusDstAlpha,
};

enum class BlendOp : u8 { add, subtract, reverseSubtract, min, max };

struct BlendAttachment {
    bool blendEnabled{false};
    BlendFactor srcColorBlendFactor{BlendFactor::one};
    BlendFactor dstColorBlendFactor{BlendFactor::zero};
    BlendOp colorBlendOp{BlendOp::add};
    BlendFactor srcAlphaBlendFactor{BlendFactor::one};
    BlendFactor dstAlphaBlendFactor{BlendFactor::zero};
    BlendOp alphaBlendOp{BlendOp::add};
};

struct PipelineDescription {
    ShaderHandle shader;
    RenderPassHandle renderPass;
    RasterizerDescription rasterizer;
    std::optional<DepthDescription> depth;
    std::vector<BlendAttachment> blendAttachments;
};

}  // namespace ignis::rhi
