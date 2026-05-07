#pragma once

#include <vector>

#include "ResourceHandle.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/core/Math.hh"

namespace ignis::rhi {

enum class Format {
    undefined = 0,
    r8unorm,
    r8g8unorm,
    r8g8b8unorm,
    r8g8b8a8unorm
};

enum class Layout {
    undefined = 0,
    general,
    colorAttachment,
    depthStencilAttachment,
    depthStencilReadOnly,
    shaderReadOnly,
    transferSrc,
    transferDst,
    presentSrc
};

using TextureHandle = ResourceHandle<ResourceType::texture>;

enum class TextureType { flat, cubemap };

enum class Filter { nearest, linear };

enum class Orientation { normal, flipped };

enum class TextureFlags { none = 0x0, writable = 0x1, transparent = 0x2 };

enum class Tiling { optimal = 0x0, linear = 0x1 };

enum class Repeat { repeat, mirroredRepeat, clampToEdge, clampToBorder };

enum class TextureUsage {
    none = 0x0,
    transferSrc = 0x00000001,
    transferDest = 0x00000002,
    sampled = 0x00000004,
    storage = 0x00000008,
    colorAttachment = 0x00000010,
    depthStencilAttachment = 0x00000020,
    transientAttachment = 0x00000040,
    inputAttachment = 0x00000080,
};

enum class TextureAspect : u32 {
    none = 0x0,
    color = 0x00000001,
    depth = 0x00000002,
    stencil = 0x00000004,
    metadata = 0x00000008,
};

IGNIS_BIT_ENUM(TextureUsage);
IGNIS_BIT_ENUM(TextureAspect);
IGNIS_BIT_ENUM(TextureFlags);

struct ImageDimensions {
    u32 width;
    u32 height;
    u8 channels;
};

struct TextureMetadata {
    TextureFlags flags{TextureFlags::none};
    TextureType type{TextureType::flat};
    Format format{Format::undefined};
    Tiling tiling{Tiling::optimal};
    TextureUsage usage{
        TextureUsage::transferSrc | TextureUsage::transferDest |
        TextureUsage::sampled
    };
    TextureAspect aspect{TextureAspect::color};
    u64 mipLevels{1u};
    u64 arrayLayers{1u};
};

struct SamplerProperties {
    Filter minifyFilter{Filter::nearest};
    Filter magnifyFilter{Filter::nearest};
    Repeat uRepeat{Repeat::repeat};
    Repeat vRepeat{Repeat::repeat};
    Repeat wRepeat{Repeat::repeat};
};

struct TextureDescription {
    ImageDimensions image;
    TextureMetadata metadata;
    SamplerProperties sampler;
};

Str toString(Format format);
Str toString(Tiling tiling);
Str toString(TextureUsage usage);
Str toString(TextureAspect aspect);
Str toString(TextureFlags flags);
Str toString(TextureType type);
Str toString(Filter filter);
Str toString(Repeat repeat);
Str toString(Orientation orientation);

Str toString(const ImageDimensions& dim);
Str toString(const TextureMetadata& metadata);
Str toString(const SamplerProperties& samplerProps);

}  // namespace ignis::rhi
