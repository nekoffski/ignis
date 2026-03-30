#pragma once

#include <span>

#include "RenderBundle.hh"
#include "RenderFrame.hh"
#include "RenderGraph.hh"
#include "RenderGraphLayout.hh"
#include "RenderScene.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"
#include "ignis/core/Pool.hh"
#include "ignis/rhi/Device.hh"
#include "ignis/rhi/ResourceRegistry.hh"

namespace ignis {

class Renderer : public NonCopyable, public NonMovable {
   public:
    explicit Renderer(const Config& config, Device& device,
                      ResourceRegistry& resourceRegistry);

    [[nodiscard]] Result<RenderFrame> enqueue(RenderGraphId renderGraphId,
                                              const RenderBundle& bundle) {
        return enqueue(renderGraphId,
                       std::span<const RenderBundle>{&bundle, 1});
    }

    [[nodiscard]] Result<RenderFrame> enqueue(
        RenderGraphId renderGraphId, std::span<const RenderBundle> bundles) {
        return {};
    }

    [[nodiscard]] OError wait(const RenderFrame& frame) {
        return Error::empty();
    }

    Result<RenderGraphId> compileRenderGraph(const RenderGraphLayout& layout) {
        return 0u;
    }

   private:
    Device& m_device;
    ResourceRegistry& m_resourceRegistry;

    Pool<RenderGraph> m_renderGraphs;
};

}  // namespace ignis
