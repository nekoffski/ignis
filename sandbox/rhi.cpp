#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"

int main(int argc, char** argv) {
    using namespace ignis;

    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");

    IGNIS_PROFILE_REGISTER_THREAD();

    // engine core
    Engine engine{Config::fromFile(argv[1])};
    auto& device = engine.device();

    // // create texture
    // auto textureDefinition =
    //     TextureDefinition::fromColor(Vec4{1.0f, 0.0f, 0.0f, 1.0f}, 64u, 64u);
    // auto handle = device.createTexture(textureDefinition.metadata);

    // // create staging buffer
    // const auto size = textureDefinition.pixels.size();
    // auto stagingBuffer =
    // device.createBuffer(BufferDescription::staging(size));

    // // copy data to buffer

    // // buffer -> texture

    // // texture -> buffer

    // // read back data from buffer

    // // create and submit workload
    // Workload workload{DeviceQueuetransfer};
    // auto wlReceipt = device.submit(workload);

    // if (not wlReceipt) {
    //     log::info("Failed to submit workload: {}",
    //     wlReceipt.error().message()); return -1;
    // }

    // if (auto err = device.wait(wlReceipt.value()); err) {
    //     log::info("Failed to wait for workload: {}", err->message());
    //     return -1;
    // }

    IGNIS_PROFILE_DUMP_SUMMARY();
    log::info("Cya!");
    return 0;
}
