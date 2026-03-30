#include "VKResourceRegistry.hh"

#include "VKDevice.hh"

namespace ignis {

VKResourceRegistry::VKResourceRegistry(VKDevice& device) : m_device(device) {}

BufferHandle VKResourceRegistry::createBuffer(const BufferDescription& desc) {
    // Implementation here
}

void VKResourceRegistry::destroyBuffer(BufferHandle handle) {
    // Implementation here
}

TextureHandle VKResourceRegistry::createTexture(
    const TextureDescription& desc) {
    // Implementation here
}

void VKResourceRegistry::destroyTexture(TextureHandle handle) {
    // Implementation here
}

}  // namespace ignis
