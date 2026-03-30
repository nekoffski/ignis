#include "Renderer.hh"

#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

namespace ignis {

Renderer::Renderer(const Config& config, Device& device,
                   ResourceRegistry& resourceRegistry)
    : m_device(device),
      m_resourceRegistry(resourceRegistry),
      m_renderGraphs(config.renderer().maxRenderGraphs) {}

}  // namespace ignis
