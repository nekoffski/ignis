#include "VKCommands.hh"

#include "VKBindGroup.hh"
#include "VKPipeline.hh"

namespace ignis::rhi {

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdUploadBufferToTexture& cmd
) {
    manifest.add(cmd.from).add(cmd.to);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdDownloadTextureToBuffer& cmd
) {
    manifest.add(cmd.from).add(cmd.to);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBeginRenderPass& cmd
) {
    if (cmd.attachments.size() > VKRenderPass::maxAttachments) {
        return Error{
            Error::Code::invalidArgument, "Too many attachments: {} (max {})",
            cmd.attachments.size(), VKRenderPass::maxAttachments
        };
    }

    manifest.add(cmd.renderPass);
    for (const auto& attachment : cmd.attachments) manifest.add(attachment);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdEndRenderPass& cmd
) {
    manifest.add(cmd.renderPass);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindPipeline& cmd
) {
    manifest.add(cmd.pipeline);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindBindGroup& cmd
) {
    manifest.add(cmd.bindGroup);
    return Error::empty();
}

Opt<Error> preprocessCommand(VKCommandManifest&, const CmdDraw&) {
    return Error::empty();
}

Opt<Error> preprocessCommand(VKCommandManifest&, const CmdSetViewport&) {
    return Error::empty();
}

Opt<Error> preprocessCommand(VKCommandManifest&, const CmdSetScissor&) {
    return Error::empty();
}

Opt<Error> preprocessCommand(VKCommandManifest&, const CmdDrawIndexed&) {
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindVertexBuffer& cmd
) {
    manifest.add(cmd.buffer);
    return Error::empty();
}

Opt<Error> preprocessCommand(
    VKCommandManifest& manifest, const CmdBindIndexBuffer& cmd
) {
    manifest.add(cmd.buffer);
    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdUploadBufferToTexture& cmd
) {
    auto& buffer = ctx.resource(cmd.from);
    auto& texture = ctx.resource(cmd.to);

    texture.withLayout(
        ctx.cmdBuffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        [&](VkCommandBuffer cmdBuffer) { texture.copyFrom(buffer, cmdBuffer); }
    );

    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdDownloadTextureToBuffer& cmd
) {
    auto& buffer = ctx.resource(cmd.to);
    auto& texture = ctx.resource(cmd.from);

    texture.withLayout(
        ctx.cmdBuffer(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        [&](VkCommandBuffer cmdBuffer) { texture.copyTo(buffer, cmdBuffer); }
    );

    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBeginRenderPass& cmd
) {
    std::vector<VKTexture*> attachments;
    attachments.reserve(cmd.attachments.size());

    std::transform(
        cmd.attachments.begin(), cmd.attachments.end(),
        std::back_inserter(attachments),
        [&](TextureHandle handle) { return &ctx.resource(handle); }
    );

    auto& renderPass = ctx.resource(cmd.renderPass);
    return renderPass.begin(
        ctx.cmdBuffer(), cmd.renderArea, cmd.clearColor, std::move(attachments)
    );
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdEndRenderPass& cmd
) {
    auto& renderPass = ctx.resource(cmd.renderPass);
    return renderPass.end(ctx.cmdBuffer());
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindPipeline& cmd
) {
    auto& pipeline = ctx.resource(cmd.pipeline);
    vkCmdBindPipeline(
        ctx.cmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle()
    );
    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindBindGroup& cmd
) {
    auto& bindGroup = ctx.resource(cmd.bindGroup);
    const auto& sets = bindGroup.descriptorSets();

    if (not sets.empty()) {
        vkCmdBindDescriptorSets(
            ctx.cmdBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
            bindGroup.pipelineLayout(), 0, static_cast<u32>(sets.size()),
            sets.data(), 0, nullptr
        );
    }

    const auto& pushRanges = bindGroup.pushConstantRanges();
    const auto& pushData = bindGroup.pushConstantData();
    for (const auto& range : pushRanges) {
        if (range.offset + range.size <= pushData.size()) {
            vkCmdPushConstants(
                ctx.cmdBuffer(), bindGroup.pipelineLayout(),
                toVk(range.stageFlags), range.offset, range.size,
                pushData.data() + range.offset
            );
        }
    }

    return Error::empty();
}

Opt<Error> recordCommand(const VKCommandContext& ctx, const CmdDraw& cmd) {
    vkCmdDraw(
        ctx.cmdBuffer(), cmd.vertexCount, cmd.instanceCount, cmd.firstVertex,
        cmd.firstInstance
    );
    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdSetViewport& cmd
) {
    VkViewport viewport{};
    viewport.x = cmd.area.x;
    viewport.y = cmd.area.y;
    viewport.width = cmd.area.w;
    viewport.height = cmd.area.h;
    viewport.minDepth = cmd.minDepth;
    viewport.maxDepth = cmd.maxDepth;
    vkCmdSetViewport(ctx.cmdBuffer(), 0, 1, &viewport);

    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdSetScissor& cmd
) {
    VkRect2D scissor{};
    scissor.offset = {
        static_cast<i32>(cmd.area.x), static_cast<i32>(cmd.area.y)
    };
    scissor.extent = {
        static_cast<u32>(cmd.area.w), static_cast<u32>(cmd.area.h)
    };
    vkCmdSetScissor(ctx.cmdBuffer(), 0, 1, &scissor);

    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdDrawIndexed& cmd
) {
    vkCmdDrawIndexed(
        ctx.cmdBuffer(), cmd.indexCount, cmd.instanceCount, cmd.firstIndex,
        cmd.vertexOffset, cmd.firstInstance
    );
    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindVertexBuffer& cmd
) {
    auto& buffer = ctx.resource(cmd.buffer);
    auto vkBuffer = buffer.handle();
    auto offset = cmd.offset;

    vkCmdBindVertexBuffers(ctx.cmdBuffer(), 0, 1, &vkBuffer, &offset);
    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdBindIndexBuffer& cmd
) {
    auto& buffer = ctx.resource(cmd.buffer);
    auto vkBuffer = buffer.handle();
    vkCmdBindIndexBuffer(
        ctx.cmdBuffer(), vkBuffer, cmd.offset, VK_INDEX_TYPE_UINT32
    );
    return Error::empty();
}

}  // namespace ignis::rhi
