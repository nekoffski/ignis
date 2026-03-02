#pragma once

#include "CommandList.hh"
#include "FrameStatistics.hh"
#include "ember/core/Concepts.hh"
#include "ember/core/Core.hh"
#include "ember/core/Error.hh"
#include "ember/rhi/RHI.hh"

namespace ember {

template <typename Callback>
concept RenderCallback = requires(Callback&& callback, CommandList cl) {
    { std::forward<Callback>(callback)(cl) } -> std::same_as<void>;
};

class Renderer : public NonCopyable, public NonMovable {
   public:
    explicit Renderer(RHI& rhi);

    template <typename Callback>
        requires RenderCallback<Callback>
    Result<FrameStatistics> renderFrame(Callback&& callback) {
        FrameStatistics stats;

        CommandList commandList{CommandList::Guard{}};
        callback(commandList);

        return stats;
    }

   private:
    RHI& m_rhi;
};

}  // namespace ember
