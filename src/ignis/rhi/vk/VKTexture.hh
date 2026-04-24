#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Scope.hh"
#include "ignis/rhi/DeviceQueue.hh"
#include "ignis/rhi/DeviceTexture.hh"

namespace ignis {

class VKDevice;
class VKCommandBuffer;
class VKBuffer;

class VKTexture : public NonCopyable {
   public:
    struct Transition {
        VkImageLayout newLayout;
        VkPipelineStageFlags srcStageMask;
        VkPipelineStageFlags dstStageMask;
        VkAccessFlags srcAccessMask;
        VkAccessFlags dstAccessMask;
        DeviceQueue srcQueue{DeviceQueue::graphics};
        DeviceQueue dstQueue{DeviceQueue::graphics};
    };

    explicit VKTexture(VKDevice& device, const DeviceImageDimensions& dim,
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

    template <typename Callback>
        requires Callable<Callback, void(VkCommandBuffer)>
    void withLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout,
                    Callback&& callback) {
        auto fallback = transitionLayout(cmdBuffer, newLayout);
        ON_SCOPE_EXIT {
            if (fallback.newLayout == VK_IMAGE_LAYOUT_UNDEFINED ||
                fallback.newLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
                [[likely]] {
                return;
            }
            transitionLayout(cmdBuffer, fallback);
        };
        callback(cmdBuffer);
    }

    Transition transitionLayout(VkCommandBuffer cmdBuffer,
                                const Transition& transition);
    Transition transitionLayout(VkCommandBuffer cmdBuffer,
                                VkImageLayout newLayout);

    VKTexture& operator=(VKTexture&& other) noexcept = delete;
    VKTexture(VKTexture&& other) noexcept;

    void copyFrom(VKBuffer& buffer, VkCommandBuffer cmdBuffer);
    void copyTo(VKBuffer& buffer, VkCommandBuffer cmdBuffer);

   private:
    void bindMemory();
    void createImage(const DeviceImageDimensions& dim,
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

    DeviceImageDimensions m_dim;
    DeviceTextureMetadata m_metadata;
    DeviceSamplerProperties m_samplerProps;
};

}  // namespace ignis