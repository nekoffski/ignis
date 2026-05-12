#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Math.hh"
#include "ignis/rhi/RenderPass.hh"

namespace ignis::rhi {

class VKTexture;
class VKDevice;

class VKRenderPass : public NonCopyable {
    class VKFramebuffer : public NonCopyable {
       public:
        explicit VKFramebuffer(
            VKDevice& device, VkRenderPass renderPass,
            std::span<const VkImageView> attachments, const UVec2& size
        );
        ~VKFramebuffer();

        VkFramebuffer handle() const;

        VKFramebuffer(VKFramebuffer&&) noexcept;
        VKFramebuffer& operator=(VKFramebuffer&&) noexcept = delete;

       private:
        VKDevice& m_device;
        VkFramebuffer m_handle{VK_NULL_HANDLE};
    };

   public:
    static constexpr u8 maxAttachments = 8;

   private:
    using AttachmentHandles = std::array<VkImageView, maxAttachments>;

    struct AttachmentHandlesHash {
        size_t operator()(const VKRenderPass::AttachmentHandles& a) const {
            size_t seed = 0;
            for (auto v : a) {
                seed ^= std::hash<VkImageView>{}(v) + 0x9e3779b97f4a7c15ULL +
                        (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };

   public:
    explicit VKRenderPass(VKDevice& device, const RenderPassDescription& desc);
    void updateAttachmentLayouts();

    ~VKRenderPass();

    VkRenderPass handle() const;

    VKRenderPass(VKRenderPass&&) noexcept;
    VKRenderPass& operator=(VKRenderPass&&) noexcept;

    Opt<Error> begin(
        VkCommandBuffer cmdBuffer, const Rect<f32>& renderArea,
        std::vector<VKTexture*>&& attachments
    );
    Opt<Error> end(VkCommandBuffer cmdBuffer);

   private:
    VkFramebuffer getFramebuffer(
        std::span<VKTexture*> attachments, const UVec2& size
    );

    void create();

    VKDevice& m_device;
    RenderPassDescription m_desc;
    VkRenderPass m_handle{VK_NULL_HANDLE};

    std::vector<const Attachment*> m_attachmentDescriptions;

    std::unordered_map<AttachmentHandles, VKFramebuffer, AttachmentHandlesHash>
        m_framebuffers;

    std::optional<std::vector<VKTexture*>> m_currentAttachments;
};

}  // namespace ignis::rhi
