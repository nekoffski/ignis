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

}  // namespace ignis
