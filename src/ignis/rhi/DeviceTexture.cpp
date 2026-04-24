#include "DeviceTexture.hh"

#include <fmt/format.h>

namespace ignis {

Str toString(DeviceTextureFormat format) {
    switch (format) {
        case DeviceTextureFormat::undefined:
            return "undefined";
        case DeviceTextureFormat::r8unorm:
            return "r8unorm";
        case DeviceTextureFormat::r8g8unorm:
            return "r8g8unorm";
        case DeviceTextureFormat::r8g8b8unorm:
            return "r8g8b8unorm";
        case DeviceTextureFormat::r8g8b8a8unorm:
            return "r8g8b8a8unorm";
        default:
            return fmt::format("Unknown({})", fmt::underlying(format));
    }
}

Str toString(DeviceTextureTiling tiling) {
    switch (tiling) {
        case DeviceTextureTiling::optimal:
            return "optimal";
        case DeviceTextureTiling::linear:
            return "linear";
        default:
            return fmt::format("Unknown({})", fmt::underlying(tiling));
    }
}

Str toString(DeviceTextureUsage usage) {
    Str result;
    if (usage == DeviceTextureUsage::none) return "none";
    if (checkFlag(usage, DeviceTextureUsage::transferSrc))
        result += "transferSrc|";
    if (checkFlag(usage, DeviceTextureUsage::transferDest))
        result += "transferDest|";
    if (checkFlag(usage, DeviceTextureUsage::sampled)) result += "sampled|";
    if (checkFlag(usage, DeviceTextureUsage::storage)) result += "storage|";
    if (checkFlag(usage, DeviceTextureUsage::colorAttachment))
        result += "colorAttachment|";
    if (checkFlag(usage, DeviceTextureUsage::depthStencilAttachment))
        result += "depthStencilAttachment|";
    if (checkFlag(usage, DeviceTextureUsage::transientAttachment))
        result += "transientAttachment|";
    if (checkFlag(usage, DeviceTextureUsage::inputAttachment))
        result += "inputAttachment|";

    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(DeviceTextureAspect aspect) {
    Str result;
    if (aspect == DeviceTextureAspect::none) return "none";
    if (checkFlag(aspect, DeviceTextureAspect::color)) result += "color|";
    if (checkFlag(aspect, DeviceTextureAspect::depth)) result += "depth|";
    if (checkFlag(aspect, DeviceTextureAspect::stencil)) result += "stencil|";
    if (checkFlag(aspect, DeviceTextureAspect::metadata)) result += "metadata|";
    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(DeviceTextureFlags flags) {
    Str result;
    if (flags == DeviceTextureFlags::none) return "none";
    if (checkFlag(flags, DeviceTextureFlags::writable)) result += "writable|";
    if (checkFlag(flags, DeviceTextureFlags::transparent))
        result += "transparent|";
    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(DeviceTextureType type) {
    switch (type) {
        case DeviceTextureType::flat:
            return "flat";
        case DeviceTextureType::cubemap:
            return "cubemap";
        default:
            return fmt::format("Unknown({})", fmt::underlying(type));
    }
}

Str toString(DeviceTextureFilter filter) {
    switch (filter) {
        case DeviceTextureFilter::nearest:
            return "nearest";
        case DeviceTextureFilter::linear:
            return "linear";
        default:
            return fmt::format("Unknown({})", fmt::underlying(filter));
    }
}

Str toString(DeviceTextureRepeat repeat) {
    switch (repeat) {
        case DeviceTextureRepeat::repeat:
            return "repeat";
        case DeviceTextureRepeat::mirroredRepeat:
            return "mirroredRepeat";
        case DeviceTextureRepeat::clampToEdge:
            return "clampToEdge";
        case DeviceTextureRepeat::clampToBorder:
            return "clampToBorder";
        default:
            return fmt::format("Unknown({})", fmt::underlying(repeat));
    }
}

Str toString(DeviceTextureOrientation orientation) {
    switch (orientation) {
        case DeviceTextureOrientation::normal:
            return "normal";
        case DeviceTextureOrientation::flipped:
            return "flipped";
        default:
            return fmt::format("Unknown({})", fmt::underlying(orientation));
    }
}

Str toString(const DeviceImageDimensions& dim) {
    return fmt::format("{}x{}x{}", dim.width, dim.height, dim.channels);
}

Str toString(const DeviceTextureMetadata& metadata) {
    return fmt::format(
        "flags: {}, type: {}, format: {}, tiling: {}, usage: {}, aspect: {}, "
        "mipLevels: {}, arrayLayers: {}",
        toString(metadata.flags), toString(metadata.type),
        toString(metadata.format), toString(metadata.tiling),
        toString(metadata.usage), toString(metadata.aspect), metadata.mipLevels,
        metadata.arrayLayers);
}

Str toString(const DeviceSamplerProperties& samplerProps) {
    return fmt::format(
        "minifyFilter: {}, magnifyFilter: {}, uRepeat: {}, vRepeat: {}, "
        "wRepeat: {}",
        toString(samplerProps.minifyFilter),
        toString(samplerProps.magnifyFilter), toString(samplerProps.uRepeat),
        toString(samplerProps.vRepeat), toString(samplerProps.wRepeat));
}

}  // namespace ignis
