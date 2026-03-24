#include "ResourceRegistry.hh"

#include "vk/VulkanDevice.hh"
#include "vk/VulkanResourceRegistry.hh"

namespace ignis {

std::unique_ptr<ResourceRegistry> ResourceRegistry::create(const Config& config,
                                                           Device& device) {
    const auto& backend = config.renderer().backend;

    if (backend == Config::Renderer::Backend::vulkan) {
        auto* vkDevice = dynamic_cast<VulkanDevice*>(&device);
        log::expect(vkDevice != nullptr,
                    "Expected VulkanDevice for Vulkan backend");

        return std::make_unique<VulkanResourceRegistry>(*vkDevice);
    } else {
        log::panic("Unsupported renderer backend: {}",
                   static_cast<int>(backend));
    }
}

}  // namespace ignis
