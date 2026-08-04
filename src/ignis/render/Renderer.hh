#pragma once

#include <span>
#include <vector>

#include "Frame.hh"
#include "FrameBundle.hh"
#include "Handle.hh"
#include "RenderGraph.hh"
#include "RenderGraphLayout.hh"
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

    [[nodiscard]] Result<FrameHandle> enqueue(
        RenderGraphHandle renderGraphId, const FrameBundle& bundle
    );

    [[nodiscard]] Result<FrameHandle> enqueue(
        RenderGraphHandle renderGraphId, std::span<const FrameBundle> bundles
    );

    [[nodiscard]] Result<Frame> wait(FrameHandle frameId);

    [[nodiscard]] Result<RenderGraphHandle> compileRenderGraph(
        const RenderGraphLayout& layout
    );

   private:
    bool renderGraphLimitReached() const;

    const Config& m_config;
    rhi::Device& m_device;
    Pool<RenderGraph> m_renderGraphs;
};

}  // namespace ignis::render
