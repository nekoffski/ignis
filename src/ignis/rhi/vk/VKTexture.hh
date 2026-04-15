#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceQueue.hh"
#include "ignis/rhi/DeviceTexture.hh"

namespace ignis {

class VKDevice;
class VKCommandBuffer;

class VKTexture : public NonCopyable {
   public:
    struct Transition {
        VkImageLayout oldLayout;
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
        DeviceQueue srcQueue;
        DeviceQueue dstQueue;
    };

    explicit VKTexture(VKDevice& device, const DeviceImageProperties& imgProps,
                       const DeviceTextureMetadata& metadata,
                       const DeviceSamplerProperties& samplerProps);

    explicit VKTexture(VKDevice& device, VkImage image,
                       const DeviceTextureMetadata& metadata,
                       const DeviceSamplerProperties& samplerProps);

    ~VKTexture();

    VkImage image() const;
    VkSampler sampler() const;
    VkImageView view() const;

    bool ownedBySwapchain() const;

    VkImageLayout& layout();

    void transitionLayout(VKCommandBuffer& cmdBuffer,
                          const Transition& transition);

    VKTexture& operator=(VKTexture&& other) noexcept = delete;
    VKTexture(VKTexture&& other) noexcept;

   private:
    void bindMemory();
    void createImage(const DeviceImageProperties& imgProps,
                     const DeviceTextureMetadata& metadata);
    void createView(const DeviceTextureMetadata& metadata);
    void createSampler(const DeviceSamplerProperties& samplerProps);

    bool m_ownedBySwapchain;

    VKDevice& m_device;
    VkImage m_image{VK_NULL_HANDLE};
    VkSampler m_sampler{VK_NULL_HANDLE};
    VkImageView m_view{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};
    VkImageLayout m_layout{VK_IMAGE_LAYOUT_UNDEFINED};
};

}  // namespace ignis