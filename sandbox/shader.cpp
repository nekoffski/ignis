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
    if (not shaderDescription) {
        log::error(
            "Failed to parse shader description: {}",
            shaderDescription.error().message()
        );
        return 1;
    }

    auto shader = device.createShader(*shaderDescription);
    if (not shader) {
        log::error("Failed to create shader: {}", shader.error().message());
        return 1;
    }

    constexpr u32 width = 512u;
    constexpr u32 height = 512u;
    constexpr Format colorFormat = Format::r8g8b8a8unorm;

    TextureDescription textureDesc{};
    textureDesc.image.width = width;
    textureDesc.image.height = height;
    textureDesc.image.channels = 4u;
    textureDesc.metadata.format = colorFormat;
    textureDesc.metadata.usage =
        TextureUsage::colorAttachment | TextureUsage::transferSrc;

    auto texture = device.createTexture(textureDesc);
    if (not texture) {
        log::error("Failed to create texture: {}", texture.error().message());
        return 1;
    }

    RenderPassDescription renderPassDesc{
        .colorAttachments =
            {
                Attachment{
                    .clear = true,
                    .format = colorFormat,
                    .initialLayout = Layout::undefined,
                    .finalLayout = Layout::transferSrc,
                },
            },
        .depthAttachment = std::nullopt,
    };

    auto renderPass = device.createRenderPass(renderPassDesc);
    if (not renderPass) {
        log::error(
            "Failed to create render pass: {}", renderPass.error().message()
        );
        return 1;
    }

    PipelineDescription pipelineDesc{
        .shader = *shader,
        .renderPass = *renderPass,
        .rasterizer =
            RasterizerDescription{
                .cullMode = CullMode::none,
            },
        .depth = std::nullopt,
        .blendAttachments = {BlendAttachment{}},
    };

    auto pipeline = device.createPipeline(pipelineDesc);
    if (not pipeline) {
        log::error("Failed to create pipeline: {}", pipeline.error().message());
        return 1;
    }

    Workload workload{Queue::graphics};

    Rect<f32> renderArea{
        0.f, 0.f, static_cast<f32>(width), static_cast<f32>(height)
    };

    workload.enqueue(CmdSetViewport{.area = renderArea});
    workload.enqueue(CmdSetScissor{.area = renderArea});
    workload.enqueue(CmdBeginRenderPass{
        .renderPass = *renderPass,
        .attachments = {*texture},
        .renderArea = renderArea,
        .clearColor = {0.1f, 0.1f, 0.1f, 1.f},
    });
    workload.enqueue(CmdBindPipeline{.pipeline = *pipeline});
    workload.enqueue(CmdDraw{.vertexCount = 3});
    workload.enqueue(CmdEndRenderPass{.renderPass = *renderPass});

    auto wlReceipt = device.submit(workload);
    if (not wlReceipt) {
        log::error(
            "Failed to submit workload: {}", wlReceipt.error().message()
        );
        return 1;
    }
    if (auto err = device.wait(*wlReceipt); err) {
        log::error("Failed to wait for workload: {}", err->message());
        return 1;
    }

    auto bufferDesc = BufferDescription::staging(width * height * 4u);
    auto buffer = device.createBuffer(bufferDesc);
    if (not buffer) {
        log::error("Failed to create buffer: {}", buffer.error().message());
        return 1;
    }

    Workload downloadWorkload{Queue::transfer};
    downloadWorkload.enqueue(CmdDownloadTextureToBuffer{
        .from = *texture,
        .to = *buffer,
    });

    auto dlReceipt = device.submit(downloadWorkload);
    if (not dlReceipt) {
        log::error(
            "Failed to submit download workload: {}",
            dlReceipt.error().message()
        );
        return 1;
    }
    if (auto err = device.wait(*dlReceipt); err) {
        log::error("Failed to wait for download workload: {}", err->message());
        return 1;
    }

    std::vector<u8> readback(bufferDesc.size);
    BufferProxy{device, *buffer}.read(readback.data(), {0, bufferDesc.size});

    auto err = ImageWriter{"triangle.png"}.write({
        .pixels = readback.data(),
        .width = width,
        .height = height,
        .channels = 4u,
    });
    if (err) {
        log::error("Failed to save image: {}", err->message());
        return 1;
    }

    log::info("Saved triangle.png");

    IGNIS_PROFILE_DUMP_SUMMARY();
    return 0;
}
