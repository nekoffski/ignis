#include "VKBuffer.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

VKBuffer::VKBuffer(VKDevice& device, const BufferDescription& description)
    : m_device(device), m_description(description) {
    create();
}

VKBuffer::~VKBuffer() {
    auto device = m_device.device();
    auto allocator = m_device.allocator();

    if (m_memory) VK_TRACE(vkFreeMemory(device, m_memory, allocator));
    if (m_handle) VK_TRACE(vkDestroyBuffer(device, m_handle, allocator));
}

VKBuffer::VKBuffer(VKBuffer&& oth) noexcept
    : m_device(oth.m_device),
      m_description(oth.m_description),
      m_handle(oth.m_handle),
      m_memory(oth.m_memory),
      m_locked(oth.m_locked),
      m_memoryIndex(oth.m_memoryIndex) {
    oth.m_handle = VK_NULL_HANDLE;
    oth.m_memory = VK_NULL_HANDLE;
    oth.m_locked = false;
    oth.m_memoryIndex = -1;
}

void VKBuffer::bind(u64 offset) {
    VK_ASSERT(
        vkBindBufferMemory(m_device.device(), m_handle, m_memory, offset));
}

VkBuffer VKBuffer::handle() const { return m_handle; }
VkBuffer* VKBuffer::handlePtr() { return &m_handle; }

void VKBuffer::write(const void* data, const Range& range) {
    with(range, [&](void* ptr) { std::memcpy(ptr, data, range.size); });
}

void VKBuffer::read(void* data, const Range& range) {
    with(range, [&](void* ptr) { std::memcpy(data, ptr, range.size); });
}

void* VKBuffer::lock(const Range& range) {
    void* ptr;
    VK_ASSERT(vkMapMemory(m_device.device(), m_memory, range.offset, range.size,
                          0, &ptr));
    return ptr;
}

void VKBuffer::unlock() { vkUnmapMemory(m_device.device(), m_memory); }

void VKBuffer::create() {
    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size = m_description.size;
    bufferCreateInfo.usage = toVk(m_description.usage);

    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VK_ASSERT(vkCreateBuffer(m_device.device(), &bufferCreateInfo,
                             m_device.allocator(), &m_handle));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device.device(), m_handle,
                                  &memoryRequirements);
    auto memoryIndex = m_device.findMemoryIndex(
        memoryRequirements.memoryTypeBits, m_description.memoryProperty);
    log::expect(memoryIndex.has_value(),
                "Failed to find suitable memory type for buffer");
    m_memoryIndex = *memoryIndex;

    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = m_memoryIndex;
    VK_ASSERT(vkAllocateMemory(m_device.device(), &allocateInfo,
                               m_device.allocator(), &m_memory));

    if (m_description.bindOnCreation) bind();
}

}  // namespace ignis::rhi
