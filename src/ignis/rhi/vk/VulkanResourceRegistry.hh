#pragma once

#include "Vulkan.hh"
#include "ignis/rhi/ResourceRegistry.hh"

namespace ignis {

class VulkanDevice;

class VulkanResourceRegistry : public ResourceRegistry {
   public:
    explicit VulkanResourceRegistry(VulkanDevice& vk);

    BufferHandle createBuffer(const BufferDescription& desc) override;
    void destroyBuffer(BufferHandle handle) override;

    TextureHandle createTexture(const TextureDescription& desc) override;
    void destroyTexture(TextureHandle handle) override;

   private:
    VulkanDevice& m_vk;
};

}  // namespace ignis
