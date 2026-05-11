#include "VKRenderPass.hh"

#include <vector>

#include "VKDevice.hh"

namespace ignis::rhi {

struct CreateInfoHelper {
    CreateInfoHelper(const RenderPassDescription& desc);

    void parseAttachments(const RenderPassDescription& desc);
    void parseColorAttachment(const Attachment& attachment);
    void parseDepthAttachment(const Attachment& attachment);

    VkRenderPassCreateInfo createInfo{};
    VkSubpassDescription subpass{};
    VkSubpassDependency dependency{};

    std::vector<VkAttachmentDescription> attachmentDescriptions;

    std::vector<VkAttachmentReference> colorAttachmentReferences;
    VkAttachmentReference depthAttachmentReference{};
};

VKRenderPass::VKFramebuffer::VKFramebuffer(
    VKDevice& device, VkRenderPass renderPass,
    std::span<const VkImageView> attachments, const UVec2& size
)
    : m_device(device) {
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<u32>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size.x;
    framebufferInfo.height = size.y;
    framebufferInfo.layers = 1;

    VK_ASSERT(vkCreateFramebuffer(
        m_device.device(), &framebufferInfo, m_device.allocator(), &m_handle
    ));
}

VKRenderPass::VKFramebuffer::VKFramebuffer(VKFramebuffer&& other) noexcept
    : m_device(other.m_device), m_handle(other.m_handle) {
    other.m_handle = VK_NULL_HANDLE;
}

VKRenderPass::VKFramebuffer::~VKFramebuffer() {
    if (m_handle) {
        VK_TRACE(vkDestroyFramebuffer(
            m_device.device(), m_handle, m_device.allocator()
        ));
    }
}

VkFramebuffer VKRenderPass::VKFramebuffer::handle() const { return m_handle; }

VKRenderPass::VKRenderPass(VKDevice& device, const RenderPassDescription& desc)
    : m_device(device), m_desc(desc) {
    create();
}

VKRenderPass::~VKRenderPass() {
    if (m_handle) {
        VK_TRACE(vkDestroyRenderPass(
            m_device.device(), m_handle, m_device.allocator()
        ));
    }
}

void VKRenderPass::begin(
    VkCommandBuffer cmdBuffer, const Rect<f32>& renderArea,
    const AttachmentHandles& attachments, u8 attachmentCount
) {
    VkRenderPassBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    beginInfo.renderPass = m_handle;

    UVec2 framebufferSize{
        static_cast<u32>(renderArea.w), static_cast<u32>(renderArea.h)
    };
    beginInfo.framebuffer =
        getFramebuffer(attachments, attachmentCount, framebufferSize);

    beginInfo.renderArea.offset = {
        static_cast<i32>(renderArea.x), static_cast<i32>(renderArea.y)
    };
    beginInfo.renderArea.extent = {
        static_cast<u32>(renderArea.w), static_cast<u32>(renderArea.h)
    };

    std::vector<VkClearValue> clearValues;
    for (const auto& attachment : m_desc.colorAttachments) {
        VkClearValue clearValue{};
        if (attachment.clear) {
            clearValue.color = {{0.f, 0.f, 1.f, 1.f}};
        }
        clearValues.push_back(clearValue);
    }
    if (m_desc.depthAttachment.has_value()) {
        VkClearValue clearValue{};
        if (m_desc.depthAttachment->clear) {
            clearValue.depthStencil = {1.f, 0};
        }
        clearValues.push_back(clearValue);
    }

    beginInfo.clearValueCount = static_cast<u32>(clearValues.size());
    beginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(cmdBuffer, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKRenderPass::end(VkCommandBuffer cmdBuffer) {
    vkCmdEndRenderPass(cmdBuffer);
}

VkFramebuffer VKRenderPass::getFramebuffer(
    const AttachmentHandles& attachments, u8 attachmentCount, const UVec2& size
) {
    if (auto it = m_framebuffers.find(attachments);
        it != m_framebuffers.end()) {
        return it->second.handle();
    }

    auto [it, inserted] = m_framebuffers.emplace(
        attachments,
        VKFramebuffer{
            m_device, m_handle,
            std::span<const VkImageView>(
                attachments.begin(), attachments.begin() + attachmentCount
            ),
            size
        }
    );
    return it->second.handle();
}

VkRenderPass VKRenderPass::handle() const { return m_handle; }

void VKRenderPass::create() {
    CreateInfoHelper createInfoHelper{m_desc};
    VK_ASSERT(vkCreateRenderPass(
        m_device.device(), &createInfoHelper.createInfo, m_device.allocator(),
        &m_handle
    ));
}

VKRenderPass::VKRenderPass(VKRenderPass&& other) noexcept
    : m_device(other.m_device),
      m_desc(std::move(other.m_desc)),
      m_handle(other.m_handle) {
    other.m_handle = VK_NULL_HANDLE;
}

VKRenderPass& VKRenderPass::operator=(VKRenderPass&& other) noexcept {
    if (this != &other) {
        if (m_handle) {
            VK_TRACE(vkDestroyRenderPass(
                m_device.device(), m_handle, m_device.allocator()
            ));
        }
        m_desc = std::move(other.m_desc);
        m_handle = other.m_handle;
        other.m_handle = VK_NULL_HANDLE;
    }
    return *this;
}

CreateInfoHelper::CreateInfoHelper(const RenderPassDescription& desc) {
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

    // subpass
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;

    parseAttachments(desc);

    // dependency
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;
}

void CreateInfoHelper::parseAttachments(const RenderPassDescription& desc) {
    const auto attachmentCount =
        static_cast<u32>(desc.colorAttachments.size()) +
        (desc.depthAttachment.has_value() ? 1 : 0);

    attachmentDescriptions.reserve(attachmentCount);
    colorAttachmentReferences.reserve(attachmentCount);

    for (const auto& attachment : desc.colorAttachments)
        parseColorAttachment(attachment);

    if (desc.depthAttachment.has_value()) {
        parseDepthAttachment(desc.depthAttachment.value());
        subpass.pDepthStencilAttachment = &depthAttachmentReference;
    }

    if (not colorAttachmentReferences.empty()) {
        subpass.colorAttachmentCount =
            static_cast<u32>(colorAttachmentReferences.size());
        subpass.pColorAttachments = colorAttachmentReferences.data();
    }

    createInfo.attachmentCount =
        static_cast<u32>(attachmentDescriptions.size());
    createInfo.pAttachments = attachmentDescriptions.data();
}

void CreateInfoHelper::parseColorAttachment(const Attachment& attachment) {
    VkAttachmentDescription colorAttachmentDescription{};

    colorAttachmentDescription.format = toVk(attachment.format);
    colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDescription.loadOp = attachment.clear
                                            ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                            : VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescription.stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescription.initialLayout = toVk(attachment.initialLayout);
    colorAttachmentDescription.finalLayout = toVk(attachment.finalLayout);

    attachmentDescriptions.push_back(colorAttachmentDescription);

    VkAttachmentReference colorAttachmentReference{};
    colorAttachmentReference.attachment =
        static_cast<u32>(attachmentDescriptions.size() - 1);
    colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    colorAttachmentReferences.push_back(colorAttachmentReference);
}

void CreateInfoHelper::parseDepthAttachment(const Attachment& attachment) {
    VkAttachmentDescription depthAttachmentDescription{};

    depthAttachmentDescription.format = toVk(attachment.format);
    depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescription.loadOp = attachment.clear
                                            ? VK_ATTACHMENT_LOAD_OP_CLEAR
                                            : VK_ATTACHMENT_LOAD_OP_LOAD;
    depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDescription.stencilStoreOp =
        VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescription.initialLayout = toVk(attachment.initialLayout);
    depthAttachmentDescription.finalLayout = toVk(attachment.finalLayout);

    attachmentDescriptions.push_back(depthAttachmentDescription);

    depthAttachmentReference.attachment =
        static_cast<u32>(attachmentDescriptions.size() - 1);
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_GENERAL;
}

}  // namespace ignis::rhi
