#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/asset/Image.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Log.hh"
#include "ignis/core/Profiler.hh"
#include "ignis/core/Scope.hh"

int main(int argc, char** argv) {
    using namespace ignis;
    ON_SCOPE_EXIT { log::info("Cya!"); };

    log::init(log::LoggerOptions{.enableColors = false});
    log::expect(argc > 1, "No config file path provided");

    IGNIS_PROFILE_REGISTER_THREAD();

    // engine core
    auto config = Config::fromFile(Path{argv[1]});
    Engine engine{config};
    auto& device = engine.device();

    // create texture
    auto pixels = ImageUtils::chessboard(128, 128, 4, 16);
    u64 size = pixels.size() * sizeof(u8);

    DeviceTextureDefinition td{};
    td.image.width = 128;
    td.image.height = 128;
    td.image.channels = 4;
    td.metadata.format = DeviceTextureFormat::r8g8b8a8unorm;

    auto texture = device.createTexture(td);

    // create staging buffer
    auto bd = DeviceBufferDescription::staging(size);
    auto writeBuffer = device.createBuffer(bd);
    auto readBuffer = device.createBuffer(bd);

    DeviceBufferProxy{device, *writeBuffer}.write(
        pixels.data(), Range{.offset = 0, .size = size});

    DeviceWorkload workload{DeviceQueue::transfer};

    // buffer -> texture
    workload.enqueue(CmdUploadBufferToTexture{
        .from = *writeBuffer,
        .to = *texture,
    });

    // texture -> buffer
    workload.enqueue(CmdDownloadTextureToBuffer{
        .from = *texture,
        .to = *readBuffer,
    });

    // submit workload
    auto wlReceipt = device.submit(workload);

    if (not wlReceipt) {
        log::info("Failed to submit workload: {}", wlReceipt.error().message());
        return -1;
    }

    if (auto err = device.wait(wlReceipt.value()); err) {
        log::info("Failed to wait for workload: {}", err->message());
        return -1;
    }

    // read back data from buffer
    std::vector<u8> readback(size, 255u);

    DeviceBufferProxy{device, *readBuffer}.read(
        readback.data(), Range{.offset = 0, .size = size});

    {
        IGNIS_PROFILE_REGION("image-save");

        auto err = ImageWriter{"output.png"}.write({
            .pixels = readback.data(),
            .width = 128u,
            .height = 128u,
            .channels = 4u,
        });

        if (err.has_value()) {
            log::info("Failed to save image: {}", err->message());
            return -1;
        }
    }

    IGNIS_PROFILE_DUMP_SUMMARY();
    return 0;
}
