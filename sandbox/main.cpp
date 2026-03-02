#include <fmt/format.h>

#include "ember/core/Config.hh"
#include "ember/core/Log.hh"
#include "ember/renderer/Renderer.hh"
#include "ember/rhi/RHI.hh"

int main(int argc, char** argv) {
    ember::log::init();
    ember::log::expect(argc > 1, "No config file path provided");
    ember::Config::get().loadFromFile(argv[1]);

    auto rhi = ember::RHI::create();
    ember::Renderer renderer{*rhi};

    auto frameStats = renderer.renderFrame([&](ember::CommandList& cmd) {
        fmt::println("Hello, world!");
        return;
    });

    if (not frameStats) {
        ember::log::error("Failed to render frame: {}",
                          frameStats.error().message());
    } else {
        ember::log::info("Frame rendered successfully");
    }

    return 0;
}
