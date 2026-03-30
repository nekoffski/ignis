#pragma once

#include "VK.hh"
#include "ignis/rhi/ResourceRegistry.hh"

namespace ignis {

class VKDevice;

class VKResourceRegistry : public ResourceRegistry {
   public:
    explicit VKResourceRegistry(VKDevice& device);

    BufferHandle createBuffer(const BufferDescription& desc) override;
    void destroyBuffer(BufferHandle handle) override;

    TextureHandle createTexture(const TextureDescription& desc) override;
    void destroyTexture(TextureHandle handle) override;

   private:
    VKDevice& m_device;
};

}  // namespace ignis
