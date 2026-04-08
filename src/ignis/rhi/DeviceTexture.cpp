#include "DeviceTexture.hh"

namespace ignis {

DeviceTextureDefinition DeviceTextureDefinition::fromColor(const Vec4& color,
                                                           u32 width,
                                                           u32 height) {
    DeviceTextureDefinition definition{};
    definition.metadata.image.width = width;
    definition.metadata.image.height = height;
    definition.metadata.image.channels = 4;
    definition.metadata.view.format = DeviceTextureFormat::R8G8B8A8Unorm;
    definition.pixels.resize(width * height * 4);

    for (u32 i = 0; i < width * height; ++i) {
        definition.pixels[i * 4 + 0] = static_cast<u8>(color.r * 255);
        definition.pixels[i * 4 + 1] = static_cast<u8>(color.g * 255);
        definition.pixels[i * 4 + 2] = static_cast<u8>(color.b * 255);
        definition.pixels[i * 4 + 3] = static_cast<u8>(color.a * 255);
    }

    return definition;
}

DeviceTextureDefinition DeviceTextureDefinition::fromColor(const Vec3& color,
                                                           u32 width,
                                                           u32 height) {
    DeviceTextureDefinition definition{};
    definition.metadata.image.width = width;
    definition.metadata.image.height = height;
    definition.metadata.image.channels = 3;
    definition.metadata.view.format = DeviceTextureFormat::R8G8B8Unorm;
    definition.pixels.resize(width * height * 3);

    for (u32 i = 0; i < width * height; ++i) {
        definition.pixels[i * 3 + 0] = static_cast<u8>(color.r * 255);
        definition.pixels[i * 3 + 1] = static_cast<u8>(color.g * 255);
        definition.pixels[i * 3 + 2] = static_cast<u8>(color.b * 255);
    }

    return definition;
}

DeviceTextureDefinition DeviceTextureDefinition::fromColor(const Vec2& color,
                                                           u32 width,
                                                           u32 height) {
    DeviceTextureDefinition definition{};
    definition.metadata.image.width = width;
    definition.metadata.image.height = height;
    definition.metadata.image.channels = 2;
    definition.metadata.view.format = DeviceTextureFormat::R8G8Unorm;
    definition.pixels.resize(width * height * 2);

    for (u32 i = 0; i < width * height; ++i) {
        definition.pixels[i * 2 + 0] = static_cast<u8>(color.x * 255);
        definition.pixels[i * 2 + 1] = static_cast<u8>(color.y * 255);
    }

    return definition;
}

DeviceTextureDefinition DeviceTextureDefinition::fromColor(f32 color, u32 width,
                                                           u32 height) {
    DeviceTextureDefinition definition{};
    definition.metadata.image.width = width;
    definition.metadata.image.height = height;
    definition.metadata.image.channels = 1;
    definition.metadata.view.format = DeviceTextureFormat::R8Unorm;
    definition.pixels.resize(width * height);

    for (u32 i = 0; i < width * height; ++i)
        definition.pixels[i] = static_cast<u8>(color * 255);

    return definition;
}

}  // namespace ignis
