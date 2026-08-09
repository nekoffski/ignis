#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Scope.hh"
#include "ignis/rhi/Queue.hh"
#include "ignis/rhi/Texture.hh"

namespace ignis::rhi {

class VKDevice;
class VKCommandBuffer;
class VKBuffer;

class VKTexture : public NonCopyable {
   public:
    struct Transition {
        VkImageLayout newLayout;
        VkPipelineStageFlags2 srcStageMask;
        VkPipelineStageFlags2 dstStageMask;
        VkAccessFlags2 srcAccessMask;
        VkAccessFlags2 dstAccessMask;
        Queue srcQueue{Queue::graphics};
        Queue dstQueue{Queue::graphics};
    };

    explicit VKTexture(
        VKDevice& device, const ImageDimensions& dim,
        const TextureMetadata& metadata, const SamplerProperties& samplerProps
    );

    explicit VKTexture(
        VKDevice& device, VkImage image, const TextureMetadata& metadata,
        const SamplerProperties& samplerProps
    );

    ~VKTexture();

    VkImage image() const;
    VkSampler sampler() const;
    VkImageView view() const;

    bool ownedBySwapchain() const;

    VkImageLayout& layout();

    template <typename Callback>
        requires Callable<Callback, void(VkCommandBuffer)>
    void withLayout(
        VkCommandBuffer cmdBuffer, VkImageLayout newLayout, Callback&& callback
    ) {
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

    Transition transitionLayout(
        VkCommandBuffer cmdBuffer, const Transition& transition
    );
    Transition transitionLayout(
        VkCommandBuffer cmdBuffer, VkImageLayout newLayout
    );

    VKTexture& operator=(VKTexture&& other) noexcept = delete;
    VKTexture(VKTexture&& other) noexcept;

    void copyFrom(VKBuffer& buffer, VkCommandBuffer cmdBuffer);
    void copyTo(VKBuffer& buffer, VkCommandBuffer cmdBuffer);

   private:
    void bindMemory();
    void createImage(
        const ImageDimensions& dim, const TextureMetadata& metadata
    );
    void createView(const TextureMetadata& metadata);
    void createSampler(const SamplerProperties& samplerProps);

    bool m_ownedBySwapchain;

    VKDevice& m_device;
    VkImage m_image{VK_NULL_HANDLE};
    VkSampler m_sampler{VK_NULL_HANDLE};
    VkImageView m_view{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};
    VkImageLayout m_layout{VK_IMAGE_LAYOUT_UNDEFINED};

    ImageDimensions m_dim;
    TextureMetadata m_metadata;
    SamplerProperties m_samplerProps;
};

}  // namespace ignis::rhi
