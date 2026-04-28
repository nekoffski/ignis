#pragma once

#include <span>

#include "Bundle.hh"
#include "Frame.hh"
#include "Graph.hh"
#include "GraphLayout.hh"
#include "Scene.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"
#include "ignis/core/Pool.hh"
#include "ignis/rhi/Device.hh"

namespace ignis::render {

class Renderer : public NonCopyable, public NonMovable {
   public:
    explicit Renderer(const Config& config, rhi::Device& device);

    // [[nodiscard]] Result<RenderFrame> enqueue(RenderGraphId renderGraphId,
    //                                           const RenderBundle& bundle) {
    //     return enqueue(renderGraphId,
    //                    std::span<const RenderBundle>{&bundle, 1});
    // }

    // [[nodiscard]] Result<RenderFrame> enqueue(
    //     RenderGraphId renderGraphId, std::span<const RenderBundle> bundles) {
    //     return {};
    // }

    // [[nodiscard]] Opt<Error> wait(const RenderFrame& frame) {
    //     return Error::empty();
    // }

    // Result<RenderGraphId> compileRenderGraph(const RenderGraphLayout& layout)
    // {
    //     return 0u;
    // }

   private:
    rhi::Device& m_device;
    // Pool<render::Graph> m_renderGraphs;
};

}  // namespace ignis::render
