#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceTexture.hh"

namespace ignis {

class VKDevice;

class VKTexture : public NonCopyable, public NonMovable {
   public:
    explicit VKTexture(VKDevice& device, const DeviceImageProperties& imgProps,
                       const DeviceViewProperties& viewProps,
                       const DeviceSamplerProperties& samplerProps);

    explicit VKTexture(VKDevice& device, VkImage image,
                       const DeviceViewProperties& viewProps,
                       const DeviceSamplerProperties& samplerProps);

    ~VKTexture();

    VkImage image() const;
    VkSampler sampler() const;
    VkImageView view() const;

    bool ownedBySwapchain() const;

    VkImageLayout& layout();

   private:
    void bindMemory();
    void createImage(const DeviceImageProperties& imgProps,
                     const DeviceViewProperties& viewProps);
    void createView(const DeviceViewProperties& viewProps);
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