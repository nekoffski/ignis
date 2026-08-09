#include <gtest/gtest.h>

#include <array>

#include "ignis/asset/Image.hh"
#include "ignis/asset/ShaderParser.hh"
#include "ignis/core/Config.hh"
#include "ignis/render/Vertex.hh"
#include "ignis/rhi/Command.hh"
#include "ignis/rhi/Device.hh"

namespace {

const ignis::Config& testConfig() {
    static const auto config = ignis::Config::fromFile(IGNIS_TEST_CONFIG_PATH);
    return config;
}

std::unique_ptr<ignis::rhi::Device> createHeadlessDevice() {
    return ignis::rhi::Device::create(testConfig());
}

}  // namespace

TEST(HeadlessRhiTest, ReportsBackendNeutralDeviceCapabilities) {
    auto device = createHeadlessDevice();

    const auto& capabilities = device->capabilities();

    EXPECT_FALSE(capabilities.deviceName.empty());
    EXPECT_GE(capabilities.apiVersion.major, 1);
    EXPECT_TRUE(capabilities.queues.graphics);
    EXPECT_TRUE(capabilities.queues.transfer);
    EXPECT_GT(capabilities.limits.maxImageDimension2D, 0);
    EXPECT_TRUE(capabilities.features.samplerAnisotropy);
    EXPECT_TRUE(capabilities.features.timelineSemaphores);
    EXPECT_TRUE(capabilities.features.synchronization2);
    EXPECT_TRUE(capabilities.features.dynamicRendering);
}

TEST(HeadlessRhiTest, ReturnsMonotonicTimelinePointsPerQueue) {
    auto device = createHeadlessDevice();
    ignis::rhi::Workload firstWorkload{ignis::rhi::Queue::transfer};
    ignis::rhi::Workload secondWorkload{ignis::rhi::Queue::transfer};

    const auto first = device->submit(firstWorkload);
    const auto second = device->submit(secondWorkload);

    ASSERT_TRUE(first.has_value());
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(first->queue, ignis::rhi::Queue::transfer);
    EXPECT_EQ(second->queue, ignis::rhi::Queue::transfer);
    EXPECT_LT(first->value, second->value);
    EXPECT_FALSE(device->wait(*second).has_value());
    EXPECT_FALSE(device->wait(*first).has_value());
}

TEST(HeadlessRhiTest, WaitsForCrossQueueTimelineDependencies) {
    auto device = createHeadlessDevice();
    ignis::rhi::Workload transferWorkload{ignis::rhi::Queue::transfer};
    const auto transfer = device->submit(transferWorkload);
    ASSERT_TRUE(transfer.has_value());

    ignis::rhi::Workload graphicsWorkload{ignis::rhi::Queue::graphics};
    graphicsWorkload.addDependency(*transfer);
    const auto graphics = device->submit(graphicsWorkload);

    ASSERT_TRUE(graphics.has_value());
    EXPECT_EQ(graphics->queue, ignis::rhi::Queue::graphics);
    EXPECT_FALSE(device->wait(*graphics).has_value());
}

TEST(HeadlessRhiTest, DestroyingAnInvalidHandleReturnsADomainError) {
    auto device = createHeadlessDevice();
    auto& resources = device->resources();
    const auto buffer =
        resources.create(ignis::rhi::BufferDescription::staging(4));
    ASSERT_TRUE(buffer.has_value());
    ASSERT_FALSE(resources.destroy(*buffer).has_value());

    const auto error = resources.destroy(*buffer);

    ASSERT_TRUE(error.has_value());
    EXPECT_EQ(error->code(), ignis::Error::Code::resourceMissing);
}

TEST(HeadlessRhiTest, RejectsAStaleHandleBeforeCommandRecording) {
    auto device = createHeadlessDevice();
    auto& resources = device->resources();
    const auto original =
        resources.create(ignis::rhi::BufferDescription::staging(4));
    ASSERT_TRUE(original.has_value());
    ASSERT_FALSE(resources.destroy(*original).has_value());

    const auto replacement =
        resources.create(ignis::rhi::BufferDescription::staging(4));
    ASSERT_TRUE(replacement.has_value());
    ASSERT_EQ(replacement->id, original->id);
    ASSERT_NE(replacement->generation, original->generation);

    ignis::rhi::Workload workload{ignis::rhi::Queue::graphics};
    workload.enqueue(ignis::rhi::CmdBindVertexBuffer{.buffer = *original});

    const auto receipt = device->submit(workload);

    ASSERT_FALSE(receipt.has_value());
    EXPECT_EQ(receipt.error().code(), ignis::Error::Code::resourceMissing);
    EXPECT_FALSE(resources.destroy(*replacement).has_value());
}

TEST(HeadlessRhiTest, RoundTripsTextureDataThroughStagingBuffers) {
    using namespace ignis;
    using namespace ignis::rhi;

    constexpr u32 width = 16;
    constexpr u32 height = 16;
    constexpr u32 channels = 4;
    auto expected = asset::ImageUtils::chessboard(width, height, channels, 4);

    auto device = createHeadlessDevice();
    auto& resources = device->resources();

    TextureDescription textureDescription{};
    textureDescription.image = {width, height, channels};
    textureDescription.metadata.format = Format::r8g8b8a8unorm;
    const auto texture = resources.create(textureDescription);
    ASSERT_TRUE(texture.has_value());

    const auto uploadBuffer =
        resources.create(BufferDescription::staging(expected.size()));
    const auto readbackBuffer =
        resources.create(BufferDescription::staging(expected.size()));
    ASSERT_TRUE(uploadBuffer.has_value());
    ASSERT_TRUE(readbackBuffer.has_value());
    const auto writeError = BufferProxy{resources, *uploadBuffer}.write(
        expected.data(), Range{0, expected.size()}
    );
    ASSERT_FALSE(writeError.has_value());

    Workload workload{Queue::transfer};
    workload.enqueue(
        CmdUploadBufferToTexture{
            .from = *uploadBuffer,
            .to = *texture,
        }
    );
    workload.enqueue(
        CmdDownloadTextureToBuffer{
            .from = *texture,
            .to = *readbackBuffer,
        }
    );

    const auto receipt = device->submit(workload);
    ASSERT_TRUE(receipt.has_value());
    ASSERT_FALSE(device->wait(*receipt).has_value());

    std::vector<u8> actual(expected.size());
    const auto readError = BufferProxy{resources, *readbackBuffer}.read(
        actual.data(), Range{0, actual.size()}
    );
    ASSERT_FALSE(readError.has_value());

    EXPECT_EQ(actual, expected);
}

TEST(HeadlessRhiTest, ClearsAndReadsBackAColorAttachment) {
    using namespace ignis;
    using namespace ignis::rhi;

    constexpr u32 width = 16;
    constexpr u32 height = 16;
    constexpr u32 channels = 4;

    auto device = createHeadlessDevice();
    auto& resources = device->resources();

    TextureDescription textureDescription{};
    textureDescription.image = {width, height, channels};
    textureDescription.metadata.format = Format::r8g8b8a8unorm;
    textureDescription.metadata.usage =
        TextureUsage::colorAttachment | TextureUsage::transferSrc;
    const auto texture = resources.create(textureDescription);
    ASSERT_TRUE(texture.has_value());

    const auto renderPass = resources.create(
        RenderPassDescription{
            .colorAttachments = {Attachment{
                .clear = true,
                .format = textureDescription.metadata.format,
                .initialLayout = Layout::undefined,
                .finalLayout = Layout::transferSrc,
            }},
            .depthAttachment = std::nullopt,
        }
    );
    ASSERT_TRUE(renderPass.has_value());

    Workload clearWorkload{Queue::graphics};
    clearWorkload.enqueue(
        CmdBeginRenderPass{
            .renderPass = *renderPass,
            .attachments = {*texture},
            .renderArea = {0, 0, width, height},
            .clearColor = {0, 1, 0, 1},
        }
    );
    clearWorkload.enqueue(CmdEndRenderPass{.renderPass = *renderPass});

    const auto clearReceipt = device->submit(clearWorkload);
    ASSERT_TRUE(clearReceipt.has_value());

    const auto readbackBuffer =
        resources.create(BufferDescription::staging(width * height * channels));
    ASSERT_TRUE(readbackBuffer.has_value());

    Workload readbackWorkload{Queue::transfer};
    readbackWorkload.addDependency(*clearReceipt);
    readbackWorkload.enqueue(
        CmdDownloadTextureToBuffer{
            .from = *texture,
            .to = *readbackBuffer,
        }
    );
    const auto readbackReceipt = device->submit(readbackWorkload);
    ASSERT_TRUE(readbackReceipt.has_value());
    ASSERT_FALSE(device->wait(*readbackReceipt).has_value());

    std::vector<u8> actual(width * height * channels);
    const auto readError = BufferProxy{resources, *readbackBuffer}.read(
        actual.data(), Range{0, actual.size()}
    );
    ASSERT_FALSE(readError.has_value());

    std::vector<u8> expected(actual.size());
    for (u64 pixel = 0; pixel < width * height; ++pixel) {
        expected[pixel * channels + 0] = 0;
        expected[pixel * channels + 1] = 255;
        expected[pixel * channels + 2] = 0;
        expected[pixel * channels + 3] = 255;
    }
    EXPECT_EQ(actual, expected);
}

TEST(HeadlessRhiTest, DrawsAnIndexedTriangle) {
    using namespace ignis;
    using namespace ignis::asset;
    using namespace ignis::render;
    using namespace ignis::rhi;

    constexpr u32 width = 64;
    constexpr u32 height = 64;
    constexpr u32 channels = 4;

    auto device = createHeadlessDevice();
    auto& resources = device->resources();

    const std::array<Vertex, 3> vertices{{
        {{-0.5f, -0.5f, 0}, {1, 0, 0}},
        {{0.5f, 0.5f, 0}, {0, 1, 0}},
        {{-0.5f, 0.5f, 0}, {0, 0, 1}},
    }};
    const std::array<u32, 3> indices{0, 1, 2};

    const auto vertexBuffer = resources.create(
        BufferDescription{
            .size = sizeof(vertices),
            .usage = BufferUsage::vertexBuffer,
            .memoryProperty =
                MemoryProperty::hostVisible | MemoryProperty::hostCoherent,
        }
    );
    const auto indexBuffer = resources.create(
        BufferDescription{
            .size = sizeof(indices),
            .usage = BufferUsage::indexBuffer,
            .memoryProperty =
                MemoryProperty::hostVisible | MemoryProperty::hostCoherent,
        }
    );
    ASSERT_TRUE(vertexBuffer.has_value());
    ASSERT_TRUE(indexBuffer.has_value());
    const auto vertexWriteError = BufferProxy{resources, *vertexBuffer}.write(
        vertices.data(), Range{0, sizeof(vertices)}
    );
    const auto indexWriteError = BufferProxy{resources, *indexBuffer}.write(
        indices.data(), Range{0, sizeof(indices)}
    );
    ASSERT_FALSE(vertexWriteError.has_value());
    ASSERT_FALSE(indexWriteError.has_value());

    const auto shaderDescription =
        ShaderParser{testConfig()}.parseFile("triangle.igshader");
    ASSERT_TRUE(shaderDescription.has_value());
    const auto shader = resources.create(*shaderDescription);
    ASSERT_TRUE(shader.has_value());
    const auto bindGroup = resources.create(BindGroupDescription{*shader});
    ASSERT_TRUE(bindGroup.has_value());

    TextureDescription textureDescription{};
    textureDescription.image = {width, height, channels};
    textureDescription.metadata.format = Format::r8g8b8a8unorm;
    textureDescription.metadata.usage =
        TextureUsage::colorAttachment | TextureUsage::transferSrc;
    const auto texture = resources.create(textureDescription);
    ASSERT_TRUE(texture.has_value());

    const auto renderPass = resources.create(
        RenderPassDescription{
            .colorAttachments = {Attachment{
                .clear = true,
                .format = textureDescription.metadata.format,
                .initialLayout = Layout::undefined,
                .finalLayout = Layout::transferSrc,
            }},
            .depthAttachment = std::nullopt,
        }
    );
    ASSERT_TRUE(renderPass.has_value());

    const auto pipeline = resources.create(
        PipelineDescription{
            .shader = *shader,
            .renderPass = *renderPass,
            .rasterizer = RasterizerDescription{.cullMode = CullMode::none},
            .depth = std::nullopt,
            .blendAttachments = {BlendAttachment{}},
        }
    );
    ASSERT_TRUE(pipeline.has_value());

    const Rect<f32> renderArea{
        0, 0, static_cast<f32>(width), static_cast<f32>(height)
    };
    Workload drawWorkload{Queue::graphics};
    drawWorkload.enqueue(CmdSetViewport{.area = renderArea});
    drawWorkload.enqueue(CmdSetScissor{.area = renderArea});
    drawWorkload.enqueue(
        CmdBeginRenderPass{
            .renderPass = *renderPass,
            .attachments = {*texture},
            .renderArea = renderArea,
            .clearColor = {0.1f, 0.1f, 0.1f, 1},
        }
    );

    const Vec4 tint{1, 1, 1, 1};
    const auto bindError = BindGroupProxy{resources, *bindGroup}.set(
        BindGroupProxy::PushConstantLocation{0, sizeof(tint)}, &tint
    );
    ASSERT_FALSE(bindError.has_value());
    drawWorkload.enqueue(CmdBindPipeline{.pipeline = *pipeline});
    drawWorkload.enqueue(CmdBindBindGroup{.bindGroup = *bindGroup});
    drawWorkload.enqueue(CmdBindVertexBuffer{.buffer = *vertexBuffer});
    drawWorkload.enqueue(CmdBindIndexBuffer{.buffer = *indexBuffer});
    drawWorkload.enqueue(
        CmdDrawIndexed{.indexCount = static_cast<u32>(indices.size())}
    );
    drawWorkload.enqueue(CmdEndRenderPass{.renderPass = *renderPass});

    const auto drawReceipt = device->submit(drawWorkload);
    ASSERT_TRUE(drawReceipt.has_value());
    ASSERT_FALSE(device->wait(*drawReceipt).has_value());

    const auto readbackBuffer =
        resources.create(BufferDescription::staging(width * height * channels));
    ASSERT_TRUE(readbackBuffer.has_value());
    Workload readbackWorkload{Queue::transfer};
    readbackWorkload.enqueue(
        CmdDownloadTextureToBuffer{
            .from = *texture,
            .to = *readbackBuffer,
        }
    );
    const auto readbackReceipt = device->submit(readbackWorkload);
    ASSERT_TRUE(readbackReceipt.has_value());
    ASSERT_FALSE(device->wait(*readbackReceipt).has_value());

    std::vector<u8> pixels(width * height * channels);
    const auto readError = BufferProxy{resources, *readbackBuffer}.read(
        pixels.data(), Range{0, pixels.size()}
    );
    ASSERT_FALSE(readError.has_value());

    const std::array<u8, channels> background{26, 26, 26, 255};
    u64 shadedPixelCount = 0;
    for (u64 pixel = 0; pixel < width * height; ++pixel) {
        const auto offset = pixel * channels;
        const std::array<u8, channels> color{
            pixels[offset], pixels[offset + 1], pixels[offset + 2],
            pixels[offset + 3]
        };
        shadedPixelCount += color != background;
    }

    EXPECT_EQ(
        (std::array<u8, channels>{pixels[0], pixels[1], pixels[2], pixels[3]}),
        background
    );
    EXPECT_GT(shadedPixelCount, width * height / 16);
    EXPECT_LT(shadedPixelCount, width * height / 4);
}
