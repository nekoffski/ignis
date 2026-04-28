#include "Engine.hh"

#include "ignis/core/ServiceLocator.hh"

namespace ignis {

Engine::Engine(const Config& config)
    : m_config(config),
      m_device(rhi::Device::create(config)),
      m_renderer(config, *m_device) {
    ServiceLocator<Config>::set(&m_config);
    ServiceLocator<rhi::Device>::set(m_device.get());
}

Engine::~Engine() {
    ServiceLocator<rhi::Device>::clear();
    ServiceLocator<Config>::clear();
}

render::Renderer& Engine::renderer() { return m_renderer; }

rhi::Device& Engine::device() { return *m_device; }

}  // namespace ignis
