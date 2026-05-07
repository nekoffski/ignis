#include <fmt/format.h>

#include "ignis/Engine.hh"
#include "ignis/asset/Image.hh"
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

    // engine core
    auto config = Config::fromFile(Path{argv[1]});
    Engine engine{config};
    auto& device = engine.device();

    // create texture to render
    TextureDescription textureDesc{};
    textureDesc.image.width = 128u;
    textureDesc.image.height = 128u;
    textureDesc.image.channels = 4u;
    textureDesc.metadata.format = Format::r8g8b8a8unorm;
    textureDesc.metadata.usage =
        TextureUsage::colorAttachment | TextureUsage::transferSrc;

    auto texture = device.createTexture(textureDesc);

    // create render pass
    RenderPassDescription renderPassDesc{
        .colorAttachments =
            {
                Attachment{
                    .clear = true,
                    .format = textureDesc.metadata.format,
                    .initialLayout = Layout::undefined,
                    .finalLayout = Layout::transferSrc,
                },
            },
        .depthAttachment = std::nullopt,

    };
    auto renderPass = device.createRenderPass(renderPassDesc);

    Workload workload{Queue::graphics};

    workload.enqueue(CmdBeginRenderPass{
        .renderPass = *renderPass,
        .attachments = {texture.value()},
        .renderArea = {.x = 0.f, .y = 0.f, .w = 128.f, .h = 128.f},
        .clearColor = {0.f, 1.f, 0.f, 1.f},
    });

    workload.enqueue(CmdEndRenderPass{
        .renderPass = *renderPass,
    });

    std::vector<Workload*> workloads;
    workloads.push_back(&workload);

    // create buffer for readback
    auto bufferDesc = BufferDescription::staging(128u * 128u * 4u);
    auto buffer = device.createBuffer(bufferDesc);

    Workload downloadWorkload{Queue::transfer};
    downloadWorkload.enqueue(CmdDownloadTextureToBuffer{
        .from = *texture,
        .to = *buffer,
    });
    workloads.push_back(&downloadWorkload);

    for (const auto& workload : workloads) {
        auto wlReceipt = device.submit(*workload);

        if (not wlReceipt) {
            log::info(
                "Failed to submit workload: {}", wlReceipt.error().message()
            );
            return -1;
        }

        if (auto err = device.wait(wlReceipt.value()); err) {
            log::info("Failed to wait for workload: {}", err->message());
            return -1;
        }
    }

    // copy data to buffer
    std::vector<u8> readback(bufferDesc.size);
    BufferProxy{device, *buffer}.read(readback.data(), {0, bufferDesc.size});

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
