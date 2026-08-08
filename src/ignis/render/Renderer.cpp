#include "Renderer.hh"

#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

namespace ignis::render {

Renderer::Renderer(const Config& config, rhi::Device& device)
    : m_config(config),
      m_device(device),
      m_renderGraphs(config.renderer().maxRenderGraphs) {}

Result<RenderGraphHandle> Renderer::compileRenderGraph(
    const RenderGraphLayout& layout
) {
    return RenderGraphHandle{};
}

Result<FrameHandle> Renderer::enqueue(
    RenderGraphHandle renderGraphId, const FrameBundle& bundle
) {
    return enqueue(renderGraphId, std::span<const FrameBundle>{&bundle, 1});
}

Result<FrameHandle> Renderer::enqueue(
    RenderGraphHandle renderGraphId, std::span<const FrameBundle> bundles
) {
    return {};
}

Result<Frame> Renderer::wait(FrameHandle frameHandle) { return {}; }

bool Renderer::renderGraphLimitReached() const {
    return m_config.renderer().maxRenderGraphs <= m_renderGraphs.liveCount();
}

}  // namespace ignis::render
