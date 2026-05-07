#pragma once

#include <vulkan/vulkan.h>

#include "ignis/core/Log.hh"
#include "ignis/rhi/Buffer.hh"
#include "ignis/rhi/Texture.hh"

namespace ignis::rhi {

using Allocator = const VkAllocationCallbacks*;

#define VK_ASSERT(x)                                                   \
    do {                                                               \
        log::trace("Vulkan call: {}", #x);                             \
        auto res = (x);                                                \
        if (res != VK_SUCCESS) {                                       \
            log::panic(                                                \
                "Vulkan error: {} at {}:{}: {}", fmt::underlying(res), \
                __FILE__, __LINE__, #x                                 \
            );                                                         \
        }                                                              \
    } while (0)

#define VK_TRACE(x)                        \
    do {                                   \
        log::trace("Vulkan call: {}", #x); \
        (x);                               \
    } while (0)

std::string toString(VkResult result, bool extended = true);

VkCommandBufferUsageFlags toVk(BufferUsage usage);
VkMemoryPropertyFlags toVk(MemoryProperty memoryProperty);
VkFormat toVk(Format format);
VkImageTiling toVk(Tiling tiling);
VkImageUsageFlags toVk(TextureUsage usage);
VkImageAspectFlags toVk(TextureAspect aspect);
VkFilter toVk(Filter filter);
VkSamplerAddressMode toVk(Repeat repeat);
VkImageLayout toVk(Layout layout);

Format fromVk(VkFormat format);

}  // namespace ignis::rhi
