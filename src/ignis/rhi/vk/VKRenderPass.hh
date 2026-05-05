#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Math.hh"
#include "ignis/rhi/RenderPass.hh"

namespace ignis::rhi {

class VKDevice;

class VKRenderPass : public NonCopyable {
    class VKFramebuffer : public NonCopyable {
       public:
        explicit VKFramebuffer(VKDevice& device, VkRenderPass renderPass,
                               std::span<VkImageView> attachments,
                               const UVec2& size);
        ~VKFramebuffer();

        VkFramebuffer handle() const;

        VKFramebuffer(VKFramebuffer&&) noexcept;
        VKFramebuffer& operator=(VKFramebuffer&&) noexcept = delete;

       private:
        VKDevice& m_device;
        VkFramebuffer m_handle{VK_NULL_HANDLE};
    };

   public:
    explicit VKRenderPass(VKDevice& device, const RenderPassDescription& desc);
    ~VKRenderPass();

    VkRenderPass handle() const;

    VKRenderPass(VKRenderPass&&) noexcept;
    VKRenderPass& operator=(VKRenderPass&&) noexcept;

    void begin(VkCommandBuffer cmdBuffer, const Rect<f32>& renderArea,
               std::span<VkImageView> attachments, u64 framebufferHash);
    void end(VkCommandBuffer cmdBuffer);

   private:
    VkFramebuffer getOrCreateFramebuffer(std::span<VkImageView> attachments,
                                         const UVec2& size,
                                         u64 framebufferHash);

    void create();

    VKDevice& m_device;
    RenderPassDescription m_desc;
    VkRenderPass m_handle{VK_NULL_HANDLE};

    std::unordered_map<u64, VKFramebuffer> m_framebuffers;
};

}  // namespace ignis::rhi
