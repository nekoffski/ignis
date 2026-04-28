#include "Texture.hh"

#include <fmt/format.h>

namespace ignis::rhi {

Str toString(TextureFormat format) {
    switch (format) {
        case TextureFormat::undefined:
            return "undefined";
        case TextureFormat::r8unorm:
            return "r8unorm";
        case TextureFormat::r8g8unorm:
            return "r8g8unorm";
        case TextureFormat::r8g8b8unorm:
            return "r8g8b8unorm";
        case TextureFormat::r8g8b8a8unorm:
            return "r8g8b8a8unorm";
        default:
            return fmt::format("Unknown({})", fmt::underlying(format));
    }
}

Str toString(TextureTiling tiling) {
    switch (tiling) {
        case TextureTiling::optimal:
            return "optimal";
        case TextureTiling::linear:
            return "linear";
        default:
            return fmt::format("Unknown({})", fmt::underlying(tiling));
    }
}

Str toString(TextureUsage usage) {
    Str result;
    if (usage == TextureUsage::none) return "none";
    if (checkFlag(usage, TextureUsage::transferSrc))
        result += "transferSrc|";
    if (checkFlag(usage, TextureUsage::transferDest))
        result += "transferDest|";
    if (checkFlag(usage, TextureUsage::sampled)) result += "sampled|";
    if (checkFlag(usage, TextureUsage::storage)) result += "storage|";
    if (checkFlag(usage, TextureUsage::colorAttachment))
        result += "colorAttachment|";
    if (checkFlag(usage, TextureUsage::depthStencilAttachment))
        result += "depthStencilAttachment|";
    if (checkFlag(usage, TextureUsage::transientAttachment))
        result += "transientAttachment|";
    if (checkFlag(usage, TextureUsage::inputAttachment))
        result += "inputAttachment|";

    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(TextureAspect aspect) {
    Str result;
    if (aspect == TextureAspect::none) return "none";
    if (checkFlag(aspect, TextureAspect::color)) result += "color|";
    if (checkFlag(aspect, TextureAspect::depth)) result += "depth|";
    if (checkFlag(aspect, TextureAspect::stencil)) result += "stencil|";
    if (checkFlag(aspect, TextureAspect::metadata)) result += "metadata|";
    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(TextureFlags flags) {
    Str result;
    if (flags == TextureFlags::none) return "none";
    if (checkFlag(flags, TextureFlags::writable)) result += "writable|";
    if (checkFlag(flags, TextureFlags::transparent))
        result += "transparent|";
    if (!result.empty()) result.pop_back();
    return result;
}

Str toString(TextureType type) {
    switch (type) {
        case TextureType::flat:
            return "flat";
        case TextureType::cubemap:
            return "cubemap";
        default:
            return fmt::format("Unknown({})", fmt::underlying(type));
    }
}

Str toString(TextureFilter filter) {
    switch (filter) {
        case TextureFilter::nearest:
            return "nearest";
        case TextureFilter::linear:
            return "linear";
        default:
            return fmt::format("Unknown({})", fmt::underlying(filter));
    }
}

Str toString(TextureRepeat repeat) {
    switch (repeat) {
        case TextureRepeat::repeat:
            return "repeat";
        case TextureRepeat::mirroredRepeat:
            return "mirroredRepeat";
        case TextureRepeat::clampToEdge:
            return "clampToEdge";
        case TextureRepeat::clampToBorder:
            return "clampToBorder";
        default:
            return fmt::format("Unknown({})", fmt::underlying(repeat));
    }
}

Str toString(TextureOrientation orientation) {
    switch (orientation) {
        case TextureOrientation::normal:
            return "normal";
        case TextureOrientation::flipped:
            return "flipped";
        default:
            return fmt::format("Unknown({})", fmt::underlying(orientation));
    }
}

Str toString(const ImageDimensions& dim) {
    return fmt::format("{}x{}x{}", dim.width, dim.height, dim.channels);
}

Str toString(const TextureMetadata& metadata) {
    return fmt::format(
        "flags: {}, type: {}, format: {}, tiling: {}, usage: {}, aspect: {}, "
        "mipLevels: {}, arrayLayers: {}",
        toString(metadata.flags), toString(metadata.type),
        toString(metadata.format), toString(metadata.tiling),
        toString(metadata.usage), toString(metadata.aspect), metadata.mipLevels,
        metadata.arrayLayers);
}

Str toString(const SamplerProperties& samplerProps) {
    return fmt::format(
        "minifyFilter: {}, magnifyFilter: {}, uRepeat: {}, vRepeat: {}, "
        "wRepeat: {}",
        toString(samplerProps.minifyFilter),
        toString(samplerProps.magnifyFilter), toString(samplerProps.uRepeat),
        toString(samplerProps.vRepeat), toString(samplerProps.wRepeat));
}

}  // namespace ignis::rhi
