#pragma once

#include <vector>

#include "ResourceHandle.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/core/Math.hh"

namespace ignis::rhi {

enum class TextureFormat : u32 {
    undefined = 0,
    r8unorm,
    r8g8unorm,
    r8g8b8unorm,
    r8g8b8a8unorm
};

using TextureHandle = ResourceHandle<ResourceType::texture>;

enum class TextureType : u8 { flat, cubemap };
enum class TextureFilter : u8 { nearest, linear };
enum class TextureOrientation : u8 { normal, flipped };

enum class TextureFlags : u8 { none = 0x0, writable = 0x1, transparent = 0x2 };

enum class TextureTiling : u8 { optimal = 0x0, linear = 0x1 };

enum class TextureRepeat : u8 {
    repeat,
    mirroredRepeat,
    clampToEdge,
    clampToBorder
};

enum class TextureUsage : u32 {
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
    TextureFormat format{TextureFormat::undefined};
    TextureTiling tiling{TextureTiling::optimal};
    TextureUsage usage{TextureUsage::transferSrc | TextureUsage::transferDest |
                       TextureUsage::sampled};
    TextureAspect aspect{TextureAspect::color};
    u64 mipLevels{1u};
    u64 arrayLayers{1u};
};

struct SamplerProperties {
    TextureFilter minifyFilter{TextureFilter::nearest};
    TextureFilter magnifyFilter{TextureFilter::nearest};
    TextureRepeat uRepeat{TextureRepeat::repeat};
    TextureRepeat vRepeat{TextureRepeat::repeat};
    TextureRepeat wRepeat{TextureRepeat::repeat};
};

struct TextureDefinition {
    ImageDimensions image;
    TextureMetadata metadata;
    SamplerProperties sampler;
};

Str toString(TextureFormat format);
Str toString(TextureTiling tiling);
Str toString(TextureUsage usage);
Str toString(TextureAspect aspect);
Str toString(TextureFlags flags);
Str toString(TextureType type);
Str toString(TextureFilter filter);
Str toString(TextureRepeat repeat);
Str toString(TextureOrientation orientation);

Str toString(const ImageDimensions& dim);
Str toString(const TextureMetadata& metadata);
Str toString(const SamplerProperties& samplerProps);

}  // namespace ignis::rhi
