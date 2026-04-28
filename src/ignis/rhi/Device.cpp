#include "Device.hh"

#include "ignis/core/Log.hh"
#include "vk/VKDevice.hh"

namespace ignis::rhi {

std::unique_ptr<Device> Device::create(const Config& config, Window* window) {
    const auto backend = config.renderer().backend;

    if (backend == Config::Renderer::Backend::vulkan) {
        return std::make_unique<VKDevice>(config, window);
    } else {
        log::panic("Unsupported renderer backend: {}",
                   static_cast<int>(backend));
    }
}

}  // namespace ignis::rhi
