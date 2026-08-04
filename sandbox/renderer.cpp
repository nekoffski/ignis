#include "ignis/render/Renderer.hh"

#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"
#include "ignis/render/FrameBundle.hh"
#include "ignis/render/RenderGraph.hh"
#include "ignis/render/RenderGraphLayout.hh"
#include "ignis/render/Scene.hh"

using namespace ignis;

class MainRenderGraphLayout : public render::RenderGraphLayout {
   public:
    struct Tags {
        render::ResourceTag colorAttachment;
    };

    MainRenderGraphLayout()
        : render::RenderGraphLayout(Name{"MainRenderGraph"}),
          m_tags{createTag()} {
        addPass(Name{"MainPass"})
            .colorAttachment(m_tags.colorAttachment)
            .clearColor({1.0f, 1.0f, 0.0f, 1.0f})
            .shader(Path{"triangle.igshader"})
            .render([](const render::FrameBundle& bundle) {
                // todo
                return;
            });
    }

    const Tags& tags() const { return m_tags; }

   private:
    Tags m_tags;
};

int main(int argc, char** argv) {
    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");

    IGNIS_PROFILE_REGISTER_THREAD();

    // engine core
    auto config = Config::fromFile(argv[1]);
    Engine engine{config};

    auto& renderer = engine.renderer();
    auto& device = engine.device();

    MainRenderGraphLayout renderGraphLayout;
    auto renderGraph = renderer.compileRenderGraph(renderGraphLayout);

    if (not renderGraph) {
        log::info(
            "Failed to compile render graph: {}", renderGraph.error().message()
        );
        return -1;
    }

    // prepare render bundle
    render::Scene scene;
    render::FrameBundle bundle;

    // enqeue bundles
    auto frame = renderer.enqueue(renderGraph.value(), bundle);

    if (not frame) {
        log::info(
            "Failed to enqueue render graph: {}", frame.error().message()
        );
        return -1;
    }

    // wait for frame to finish rendering
    if (auto result = renderer.wait(frame.value()); not result) {
        log::info("Failed to wait for frame: {}", result.error().message());
        return -1;
    }

    IGNIS_PROFILE_DUMP_SUMMARY();
    log::info("Cya!");
    return 0;
}
