// #include "RHI.hh"

// #include "ignis/core/Config.hh"
// #include "ignis/core/Log.hh"
// #include "ignis/core/ServiceLocator.hh"
// #include "vk/VKRHI.hh"

// namespace ignis {

// std::unique_ptr<RHI> RHI::create(Window* window) {
//     auto& cfg = ServiceLocator<Config>::get();

//     switch (cfg.renderer().backend) {
//         case Config::Renderer::Backend::vulkan:
//             return VKRHI::create(window);
//     }
//     log::panic("Unsupported renderer backend");
// }

// }  // namespace ignis
