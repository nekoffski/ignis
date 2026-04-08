#pragma once

#include <vulkan/vulkan.h>

#include "ignis/core/Log.hh"

namespace ignis {

using Allocator = const VkAllocationCallbacks*;

#define VK_ASSERT(x)                                                          \
    do {                                                                      \
        log::trace("Vulkan call: {}", #x);                                    \
        auto res = (x);                                                       \
        if (res != VK_SUCCESS) {                                              \
            log::panic("Vulkan error: {} at {}:{}: {}", fmt::underlying(res), \
                       __FILE__, __LINE__, #x);                               \
        }                                                                     \
    } while (0)

#define VK_TRACE(x)                        \
    do {                                   \
        log::trace("Vulkan call: {}", #x); \
        (x);                               \
    } while (0)

std::string toString(VkResult result, bool extended = true);

}  // namespace ignis
