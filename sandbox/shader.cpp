#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/asset/Image.hh"
#include "ignis/asset/ShaderParser.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"
#include "ignis/core/Scope.hh"

int main(int argc, char** argv) {
    using namespace ignis;
    using namespace ignis::rhi;
    using namespace ignis::asset;

    ON_SCOPE_EXIT { log::info("Cya!"); };

    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");
    IGNIS_PROFILE_REGISTER_THREAD();

    auto config = Config::fromFile(argv[1]);
    Engine engine{config};
    auto& device = engine.device();

    auto shaderDescription = ShaderParser{}.parseFile("triangle.igshader");

    // auto shader = device.createShader(shaderDescription);

    // if (not shader) {
    //     log::error("Failed to create shader: {}", shader.error().message());
    //     return 1;
    // }

    IGNIS_PROFILE_DUMP_SUMMARY();
    return 0;
}
