#pragma once

#include <vulkan/vulkan.h>

#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"
#include "ember/rhi/Window.hh"

namespace ember {

class VK : public NonCopyable, public NonMovable {
   public:
    explicit VK(Window* window);
    ~VK();

   private:
    void createInstance();
    void createDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();

    const VkAllocationCallbacks* allocator() const;

    Window* m_window{nullptr};

    VkInstance m_instance{VK_NULL_HANDLE};
    VkDebugUtilsMessengerEXT m_debugMessenger{VK_NULL_HANDLE};
    VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
    VkDevice m_device{VK_NULL_HANDLE};
};

#define VK_ASSERT(x)                                                          \
    do {                                                                      \
        auto res = (x);                                                       \
        if (res != VK_SUCCESS) {                                              \
            log::panic("Vulkan error: {} at {}:{}: {}", fmt::underlying(res), \
                       __FILE__, __LINE__, #x);                               \
        }                                                                     \
    } while (0)

}  // namespace ember
