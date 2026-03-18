#include "Engine.hh"

#include "ignis/core/ServiceLocator.hh"

namespace ignis {

Engine::Engine(const Config& config) : m_config(config) {
    ServiceLocator<Config>::set(&m_config);
}

Renderer& Engine::renderer() { return m_renderer; }

}  // namespace ignis
