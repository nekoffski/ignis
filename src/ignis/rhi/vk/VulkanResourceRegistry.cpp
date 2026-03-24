#include "VulkanResourceRegistry.hh"

#include "VulkanDevice.hh"

namespace ignis {

VulkanResourceRegistry::VulkanResourceRegistry(VulkanDevice& vk) : m_vk(vk) {}

BufferHandle VulkanResourceRegistry::createBuffer(
    const BufferDescription& desc) {
    // Implementation here
}

void VulkanResourceRegistry::destroyBuffer(BufferHandle handle) {
    // Implementation here
}

TextureHandle VulkanResourceRegistry::createTexture(
    const TextureDescription& desc) {
    // Implementation here
}

void VulkanResourceRegistry::destroyTexture(TextureHandle handle) {
    // Implementation here
}

}  // namespace ignis
