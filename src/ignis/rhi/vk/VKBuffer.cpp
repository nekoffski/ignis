#include "VKBuffer.hh"

#include "VKDevice.hh"

namespace ignis {

VKBuffer::VKBuffer(VKDevice& device, const DeviceBufferDescription& description)
    : m_device(device), m_description(description) {}

VKBuffer::~VKBuffer() {
    auto device = m_device.device();
    auto allocator = m_device.allocator();

    if (m_memory) VK_TRACE(vkFreeMemory(device, m_memory, allocator));
    if (m_handle) VK_TRACE(vkDestroyBuffer(device, m_handle, allocator));
}

VkBuffer VKBuffer::handle() const { return m_handle; }
VkBuffer* VKBuffer::handlePtr() { return &m_handle; }

}  // namespace ignis
