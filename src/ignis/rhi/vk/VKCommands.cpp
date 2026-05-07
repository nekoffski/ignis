#include "VKCommands.hh"

namespace ignis::rhi {

void preprocessCommand(
    VKCommandManifest& manifest, const CmdUploadBufferToTexture& cmd
) {
    manifest.add(cmd.from).add(cmd.to);
}

void preprocessCommand(
    VKCommandManifest& manifest, const CmdDownloadTextureToBuffer& cmd
) {
    manifest.add(cmd.from).add(cmd.to);
}

void preprocessCommand(
    VKCommandManifest& manifest, const CmdBeginRenderPass& cmd
) {
    manifest.add(cmd.renderPass);
    for (const auto& attachment : cmd.attachments) manifest.add(attachment);
}

void preprocessCommand(
    VKCommandManifest& manifest, const CmdEndRenderPass& cmd
) {
    manifest.add(cmd.renderPass);
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
) {}
//     auto renderPass = m_device.findRenderPass(cmd.renderPass);
//     if (not renderPass) return Error {
//             Error::Code::resourceMissing, "render pass not
//                 found "};

//                 std::vector<VkImageView>
//                     attachmentViews;
//             attachmentViews.reserve(cmd.attachments.size());

//             for (const auto& attachmentHandle : cmd.attachments) {
//                 auto texture = m_device.findTexture(attachmentHandle);
//                 if (not texture)
//                     return Error{
//                         Error::Code::resourceMissing,
//                         fmt::format(
//                             "texture not found for attachment handle "
//                             "{}",
//                             static_cast<u32>(attachmentHandle.id)
//                         )
//                     };
//                 attachmentViews.push_back(texture->view());
//             }

//             u64 attachmentHash = 0;
//             for (const auto& view : attachmentViews) {
//                 attachmentHash ^= std::hash<VkImageView>{}(view) + 0x9e3779b9
//                 +
//                                   (attachmentHash << 6) + (attachmentHash >>
//                                   2);
//             }

//             renderPass->begin(
//                 m_cmdBuffer, cmd.renderArea, attachmentViews, attachmentHash
//             );
//             return Error::empty();
//         }
// }

Opt<Error> recordCommand(
    const VKCommandContext& ctx, const CmdEndRenderPass& cmd
) {}

// Opt<Error> VKCommandDispatcher::Visitor::operator()(

// }

// Opt<Error> VKCommandDispatcher::Visitor::operator()(
//     const CmdEndRenderPass& cmd) {
//     CHECK_QUEUE(cmd, m_targetQueue);

//     auto renderPass = m_device.findRenderPass(cmd.renderPass);

//     if (not renderPass)
//         return Error{Error::Code::resourceMissing, "render pass
//         not found"};

//     renderPass->end(m_cmdBuffer);
//     return Error::empty();
// }

}  // namespace ignis::rhi
