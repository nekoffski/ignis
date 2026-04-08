#include "Renderer.hh"

#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

namespace ignis {

Renderer::Renderer(const Config& config, Device& device)
    : m_device(device), m_renderGraphs(config.renderer().maxRenderGraphs) {}

}  // namespace ignis
