#include "VKCommands.hh"

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
            Error::Code::invalidArgument,
            fmt::format(
                "Too many attachments: {} (max {})", cmd.attachments.size(),
                VKRenderPass::maxAttachments
            )
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
    VKRenderPass::AttachmentHandles attachmentViews;
    u8 attachmentCount = static_cast<u8>(cmd.attachments.size());

    for (u8 i = 0; i < attachmentCount; ++i)
        attachmentViews[i] = ctx.resource(cmd.attachments[i]).view();

    auto& renderPass = ctx.resource(cmd.renderPass);
    renderPass.begin(
        ctx.cmdBuffer(), cmd.renderArea, attachmentViews, attachmentCount
    );

    return Error::empty();
}

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdEndRenderPass& cmd
) {
    auto& renderPass = ctx.resource(cmd.renderPass);
    renderPass.end(ctx.cmdBuffer());
    return Error::empty();
}

}  // namespace ignis::rhi
