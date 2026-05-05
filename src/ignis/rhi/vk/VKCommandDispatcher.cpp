#include "VKCommandDispatcher.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

#define CHECK_QUEUE(cmd, expectedQueue)                                   \
    if (cmd.targetQueue() != expectedQueue) {                             \
        log::error("Command {} is not meant for the target queue", #cmd); \
        return Error{Error::Code::deviceQueueMismatch,                    \
                     "Command is not meant for the target queue"};        \
    }

VKCommandDispatcher::VKCommandDispatcher(VKDevice& device,
                                         VkCommandBuffer cmdBuffer,
                                         Queue targetQueue)
    : m_visitor(device, cmdBuffer, targetQueue) {}

Opt<Error> VKCommandDispatcher::dispatch(const Command& command) {
    return dispatch(std::span<const Command>{&command, 1});
}

Opt<Error> VKCommandDispatcher::dispatch(std::span<const Command> commands) {
    u64 totalCommands = commands.size();
    u64 dispatchedCommands = 0;

    Opt<Error> err = Error::empty();

    for (const auto& cmd : commands) {
        if (auto err = std::visit(m_visitor, cmd))
            err = err;
        else
            ++dispatchedCommands;
    }

    log::debug("Dispatched {} out of {} commands", dispatchedCommands,
               totalCommands);
    return err;
}

VKCommandDispatcher::Visitor::Visitor(VKDevice& device,
                                      VkCommandBuffer cmdBuffer,
                                      Queue targetQueue)
    : m_device(device), m_cmdBuffer(cmdBuffer), m_targetQueue(targetQueue) {}

Opt<Error> VKCommandDispatcher::Visitor::operator()(
    const CmdUploadBufferToTexture& cmd) {
    CHECK_QUEUE(cmd, m_targetQueue);

    auto buffer = m_device.findBuffer(cmd.from);

    if (not buffer)
        return Error{Error::Code::resourceMissing, "buffer not found"};

    auto texture = m_device.findTexture(cmd.to);
    if (not texture)
        return Error{Error::Code::resourceMissing, "texture not found"};

    texture->withLayout(m_cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        [&](VkCommandBuffer cmdBuffer) {
                            texture->copyFrom(*buffer, cmdBuffer);
                        });

    return Error::empty();
}

Opt<Error> VKCommandDispatcher::Visitor::operator()(
    const CmdDownloadTextureToBuffer& cmd) {
    CHECK_QUEUE(cmd, m_targetQueue);

    auto buffer = m_device.findBuffer(cmd.to);

    if (not buffer)
        return Error{Error::Code::resourceMissing, "buffer not found"};

    auto texture = m_device.findTexture(cmd.from);

    if (not texture)
        return Error{Error::Code::resourceMissing, "texture not found"};

    texture->withLayout(m_cmdBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        [&](VkCommandBuffer cmdBuffer) {
                            texture->copyTo(*buffer, cmdBuffer);
                        });

    return Error::empty();
}

Opt<Error> VKCommandDispatcher::Visitor::operator()(
    const CmdBeginRenderPass& cmd) {
    CHECK_QUEUE(cmd, m_targetQueue);

    auto renderPass = m_device.findRenderPass(cmd.renderPass);
    if (not renderPass)
        return Error{Error::Code::resourceMissing, "render pass not found"};

    std::vector<VkImageView> attachmentViews;
    attachmentViews.reserve(cmd.attachments.size());

    for (const auto& attachmentHandle : cmd.attachments) {
        auto texture = m_device.findTexture(attachmentHandle);
        if (not texture)
            return Error{
                Error::Code::resourceMissing,
                fmt::format("texture not found for attachment handle {}",
                            static_cast<u32>(attachmentHandle.id))};
        attachmentViews.push_back(texture->view());
    }

    u64 attachmentHash = 0;
    for (const auto& view : attachmentViews) {
        attachmentHash ^= std::hash<VkImageView>{}(view) + 0x9e3779b9 +
                          (attachmentHash << 6) + (attachmentHash >> 2);
    }

    renderPass->begin(m_cmdBuffer, cmd.renderArea, attachmentViews,
                      attachmentHash);
    return Error::empty();
}

Opt<Error> VKCommandDispatcher::Visitor::operator()(
    const CmdEndRenderPass& cmd) {
    CHECK_QUEUE(cmd, m_targetQueue);

    auto renderPass = m_device.findRenderPass(cmd.renderPass);

    if (not renderPass)
        return Error{Error::Code::resourceMissing, "render pass not found"};

    renderPass->end(m_cmdBuffer);
    return Error::empty();
}

}  // namespace ignis::rhi
