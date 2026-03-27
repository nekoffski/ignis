#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

int main(int argc, char** argv) {
    using namespace ignis;

    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");

    Profiler::get().registerThread();

    const auto config = Config::fromFile(argv[1]);

    {
        JUNO_PROFILE_REGION("engineBootstrap");

        Engine engine{config};

        auto& renderer = engine.renderer();
        auto renderGraph = renderer.createRenderGraph();

        RenderScene scene{};
        RenderView view{};

        std::vector<const RenderView*> renderViews{&view};

        if (auto err = renderer.drawFrame(*renderGraph, renderViews); err)
            log::error("Failed to draw frame: {}", err->message());
    }

    Profiler::get().generateSummary().print();

    log::info("Cya!");
    return 0;
}
