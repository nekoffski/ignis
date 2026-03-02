#include "RHI.hh"

#include "ember/core/Config.hh"
#include "ember/core/Log.hh"
#include "vk/VKRHI.hh"

namespace ember {

std::unique_ptr<RHI> RHI::create(Window* window) {
    auto& cfg = Config::get();

    switch (cfg.renderer().backend) {
        case Config::Renderer::Backend::vulkan:
            return VKRHI::create(window);
    }
    log::panic("Unsupported renderer backend");
}

}  // namespace ember
