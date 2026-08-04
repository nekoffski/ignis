#pragma once

#include <vector>

#include "FrameBundle.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Math.hh"

namespace ignis::render {

class Renderer;

class RenderGraphLayout : public NonCopyable, public NonMovable {
    using RenderCallback = std::function<void(const FrameBundle&)>;

    friend class Renderer;

    class RenderPassLayout : public NonCopyable {
        friend class Renderer;

       public:
        explicit RenderPassLayout(const Name& name);

        RenderPassLayout& colorAttachment(ResourceTag tag);
        RenderPassLayout& depthAttachment(ResourceTag tag);
        RenderPassLayout& render(RenderCallback&& callback);
        RenderPassLayout& clearColor(const Vec4& color);
        RenderPassLayout& shader(const Path& path);

       private:
        Name m_name;
    };

   public:
    explicit RenderGraphLayout(const Name& name, const Config& config);
    explicit RenderGraphLayout(const Name& name);

    const Name& name() const;

   protected:
    ResourceTag createTag();

    RenderPassLayout& addPass(const Name& name);

   private:
    u64 m_nextTag{0u};
    Name m_name;
    std::vector<RenderPassLayout> m_passes;
};

}  // namespace ignis::render
