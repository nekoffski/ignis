#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

int main(int argc, char** argv) {
    using namespace ignis;

    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");

    JUNO_PROFILE_REGISTER_THREAD();

    // engine core
    Engine engine{Config::fromFile(argv[1])};
    auto& renderer = engine.renderer();

    // compile render graph
    RGraphLayout rgraphLayout{};

    auto mainPassBody = []() { return; };

    rgraphLayout.addPass(Name{"Main Pass"}, mainPassBody)
        .access(RAcess::colorAttachment, 0)
        .access(RAcess::depthAttachment, 1);

    auto renderGraphHandle = renderer.compileRenderGraph(rgraphLayout);

    // prepare render bundle
    RScene scene{};
    RBundle bundle{};

    // enqeue bundles
    auto frame = renderer.enqueue(renderGraphHandle, bundle);

    if (not frame) {
        log::info("Failed to enqueue render graph: {}",
                  frame.error().message());
        return -1;
    }

    // wait for frame to finish rendering
    if (auto err = renderer.wait(frame.value()); err) {
        log::info("Failed to wait for frame: {}", err->message());
        return -1;
    }

    JUNO_PROFILE_DUMP_SUMMARY();
    log::info("Cya!");
    return 0;
}
