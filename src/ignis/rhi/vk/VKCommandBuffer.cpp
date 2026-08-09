#include "VKCommandBuffer.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

VKCommandBuffer::VKCommandBuffer(VKDevice& device, Queue targetQueue)
    : m_device(device), m_targetQueue(targetQueue) {
    const auto& commandPools = m_device.commandPools();

    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = targetQueue == Queue::graphics ? commandPools.graphics
                                                      : commandPools.transfer;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // for now good enough
    info.commandBufferCount = 1;

    VK_TRACE(vkAllocateCommandBuffers(m_device.device(), &info, &m_handle));
}

VKCommandBuffer::~VKCommandBuffer() {
    if (m_handle != VK_NULL_HANDLE) {
        const auto& commandPools = m_device.commandPools();
        auto pool = m_targetQueue == Queue::graphics ? commandPools.graphics
                                                     : commandPools.transfer;
        VK_TRACE(vkFreeCommandBuffers(m_device.device(), pool, 1, &m_handle));
    }
}

void VKCommandBuffer::begin(BeginFlags flags) {
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
    VK_TRACE(vkBeginCommandBuffer(m_handle, &info));
}

void VKCommandBuffer::end() { VK_TRACE(vkEndCommandBuffer(m_handle)); }

VkCommandBuffer VKCommandBuffer::handle() { return m_handle; }

VKWorkload::VKWorkload(
    VKDevice& device, Queue targetQueue, TimelinePoint completion
)
    : m_cmdBuffer(std::make_unique<VKCommandBuffer>(device, targetQueue)),
      m_completion(completion) {}

VKCommandBuffer& VKWorkload::commandBuffer() { return *m_cmdBuffer; }

TimelinePoint VKWorkload::completion() const { return m_completion; }

}  // namespace ignis::rhi
