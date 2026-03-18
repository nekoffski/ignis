#include "Renderer.hh"

namespace ignis {

std::unique_ptr<RenderGraph> Renderer::createRenderGraph() {
    return std::make_unique<RenderGraph>(RenderGraph::Guard{});
}

}  // namespace ignis