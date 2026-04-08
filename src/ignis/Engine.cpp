#include "Engine.hh"

#include "ignis/core/ServiceLocator.hh"

namespace ignis {

Engine::Engine(const Config& config)
    : m_config(config),
      m_device(Device::create(config)),
      m_renderer(config, *m_device) {
    ServiceLocator<Config>::set(&m_config);
    ServiceLocator<Device>::set(m_device.get());
}

Engine::~Engine() {
    ServiceLocator<Device>::clear();
    ServiceLocator<Config>::clear();
}

Renderer& Engine::renderer() { return m_renderer; }

Device& Engine::device() { return *m_device; }

}  // namespace ignis
