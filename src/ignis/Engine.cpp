#include "Engine.hh"

#include "ignis/core/ServiceLocator.hh"

namespace ignis {

Engine::Engine(const Config& config)
    : m_config(config),
      m_device(Device::create(config)),
      m_resourceRegistry(ResourceRegistry::create(config, *m_device)) {
    ServiceLocator<Config>::set(&m_config);
    ServiceLocator<Renderer>::set(&m_renderer);
    ServiceLocator<Device>::set(m_device.get());
    ServiceLocator<ResourceRegistry>::set(m_resourceRegistry.get());
}

Engine::~Engine() {
    ServiceLocator<ResourceRegistry>::clear();
    ServiceLocator<Device>::clear();
    ServiceLocator<Renderer>::clear();
    ServiceLocator<Config>::clear();
}

Renderer& Engine::renderer() { return m_renderer; }

Device& Engine::device() { return *m_device; }

ResourceRegistry& Engine::resourceRegistry() { return *m_resourceRegistry; }

}  // namespace ignis
