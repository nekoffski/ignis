#include "ResourceRegistry.hh"

#include "vk/VKDevice.hh"
#include "vk/VKResourceRegistry.hh"

namespace ignis {

std::unique_ptr<ResourceRegistry> ResourceRegistry::create(const Config& config,
                                                           Device& device) {
    const auto& backend = config.renderer().backend;

    if (backend == Config::Renderer::Backend::vulkan) {
        auto* vkDevice = dynamic_cast<VKDevice*>(&device);
        log::expect(vkDevice != nullptr,
                    "Expected VKDevice for Vulkan backend");

        return std::make_unique<VKResourceRegistry>(*vkDevice);
    } else {
        log::panic("Unsupported renderer backend: {}",
                   static_cast<int>(backend));
    }
}

}  // namespace ignis
