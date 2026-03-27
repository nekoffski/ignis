#include "Renderer.hh"

#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

namespace ignis {

std::unique_ptr<RenderGraph> Renderer::createRenderGraph() {
    JUNO_PROFILE_FUNCTION();

    return std::make_unique<RenderGraph>(RenderGraph::Guard{});
}

}  // namespace ignis