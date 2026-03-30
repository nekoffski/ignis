#pragma once

#include <span>

#include "RBundle.hh"
#include "RFrame.hh"
#include "RGraph.hh"
#include "RGraphLayout.hh"
#include "RScene.hh"
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

    [[nodiscard]] Result<RFrame> enqueue(RGraphId renderGraphId,
                                         const RBundle& bundle) {
        return enqueue(renderGraphId, std::span<const RBundle>{&bundle, 1});
    }

    [[nodiscard]] Result<RFrame> enqueue(RGraphId renderGraphId,
                                         std::span<const RBundle> bundles) {
        return {};
    }

    [[nodiscard]] OError wait(const RFrame& frame) { return Error::empty(); }

    RGraphId compileRenderGraph(const RGraphLayout& layout) { return 0u; }

   private:
    Device& m_device;
    ResourceRegistry& m_resourceRegistry;

    Pool<RGraph> m_renderGraphs;
};

}  // namespace ignis
