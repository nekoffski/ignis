#include "RenderGraphLayout.hh"

#include "ignis/core/ServiceLocator.hh"

namespace ignis::render {

RenderGraphLayout::RenderGraphLayout(const Name& name)
    : RenderGraphLayout(name, ServiceLocator<Config>::get()) {}

RenderGraphLayout::RenderGraphLayout(const Name& name, const Config& config)
    : m_name(name) {
    m_passes.reserve(config.renderer().maxRenderPasses);
}

const Name& RenderGraphLayout::name() const { return m_name; }

ResourceTag RenderGraphLayout::createTag() { return ResourceTag{m_nextTag++}; }

RenderGraphLayout::RenderPassLayout& RenderGraphLayout::addPass(const Name& name
) {
    m_passes.emplace_back(name);
    return m_passes.back();
}

RenderGraphLayout::RenderPassLayout::RenderPassLayout(const Name& name)
    : m_name(name) {}

RenderGraphLayout::RenderPassLayout&
RenderGraphLayout::RenderPassLayout::colorAttachment(ResourceTag tag) {
    // TODO: insert return statement here
    return *this;
}

RenderGraphLayout::RenderPassLayout&
RenderGraphLayout::RenderPassLayout::depthAttachment(ResourceTag tag) {
    // TODO: insert return statement here
    return *this;
}

RenderGraphLayout::RenderPassLayout&
RenderGraphLayout::RenderPassLayout::render(RenderCallback&& tag) {
    // TODO: insert return statement here
    return *this;
}

RenderGraphLayout::RenderPassLayout&
RenderGraphLayout::RenderPassLayout::clearColor(const Vec4& color) {
    // TODO: insert return statement here
    return *this;
}

RenderGraphLayout::RenderPassLayout&
RenderGraphLayout::RenderPassLayout::shader(const Path& path) {
    // TODO: insert return statement here
    return *this;
}

}  // namespace ignis::render
