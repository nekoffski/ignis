#pragma once

#include <vector>

#include "DeviceResourceHandle.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/core/Math.hh"

namespace ignis {

enum class DeviceTextureFormat : u32 {
    undefined = 0,
    r8unorm,
    r8g8unorm,
    r8g8b8unorm,
    r8g8b8a8unorm
};

using DeviceTextureHandle = DeviceResourceHandle<DeviceResourceType::texture>;

enum class DeviceTextureType : u8 { flat, cubemap };
enum class DeviceTextureFilter : u8 { nearest, linear };
enum class DeviceTextureOrientation : u8 { normal, flipped };

enum class DeviceTextureFlags : u8 {
    none = 0x0,
    writable = 0x1,
    transparent = 0x2
};

enum class DeviceTextureTiling : u8 { optimal = 0x0, linear = 0x1 };

enum class DeviceTextureRepeat : u8 {
    repeat,
    mirroredRepeat,
    clampToEdge,
    clampToBorder
};

enum class DeviceTextureUsage : u32 {
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

enum class DeviceTextureAspect : u32 {
    none = 0x0,
    color = 0x00000001,
    depth = 0x00000002,
    stencil = 0x00000004,
    metadata = 0x00000008,
};

IGNIS_BIT_ENUM(DeviceTextureUsage);
IGNIS_BIT_ENUM(DeviceTextureAspect);

struct DeviceImageDimensions {
    u32 width;
    u32 height;
    u8 channels;
};

struct DeviceTextureMetadata {
    DeviceTextureFlags flags{DeviceTextureFlags::none};
    DeviceTextureType type{DeviceTextureType::flat};
    DeviceTextureFormat format{DeviceTextureFormat::undefined};
    DeviceTextureTiling tiling{DeviceTextureTiling::optimal};
    DeviceTextureUsage usage{DeviceTextureUsage::transferSrc |
                             DeviceTextureUsage::transferDest |
                             DeviceTextureUsage::sampled};
    DeviceTextureAspect aspect{DeviceTextureAspect::color};
    u64 mipLevels{1u};
    u64 arrayLayers{1u};
};

struct DeviceSamplerProperties {
    DeviceTextureFilter minifyFilter{DeviceTextureFilter::nearest};
    DeviceTextureFilter magnifyFilter{DeviceTextureFilter::nearest};
    DeviceTextureRepeat uRepeat{DeviceTextureRepeat::repeat};
    DeviceTextureRepeat vRepeat{DeviceTextureRepeat::repeat};
    DeviceTextureRepeat wRepeat{DeviceTextureRepeat::repeat};
};

struct DeviceTextureDefinition {
    DeviceImageDimensions image;
    DeviceTextureMetadata metadata;
    DeviceSamplerProperties sampler;
};

Str toString(DeviceTextureFormat format);
Str toString(DeviceTextureTiling tiling);
Str toString(DeviceTextureUsage usage);
Str toString(DeviceTextureAspect aspect);

}  // namespace ignis
