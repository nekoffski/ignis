#pragma once

#include <span>

#include "RenderGraph.hh"
#include "RenderScene.hh"
#include "RenderView.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Error.hh"

namespace ignis {

class Renderer : public NonCopyable, public NonMovable {
   public:
    std::unique_ptr<RenderGraph> createRenderGraph();

    [[nodiscard]] OError drawFrame(const RenderGraph& renderGraph,
                                   std::span<const RenderView*> renderViews) {}

   private:
};

}  // namespace ignis
