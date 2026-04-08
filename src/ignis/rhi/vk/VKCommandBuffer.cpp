#include "VKCommandBuffer.hh"

#include "VKCommands.hh"
#include "VKDevice.hh"

namespace ignis {

VKCommandBuffer::VKCommandBuffer(VKDevice& device) : m_device(device) {
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.commandPool = m_device.graphicsCommandPool();
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  // for now good enough
    info.commandBufferCount = 1;

    VK_TRACE(vkAllocateCommandBuffers(m_device.device(), &info, &m_handle));
}

VKCommandBuffer::~VKCommandBuffer() {
    if (m_handle != VK_NULL_HANDLE) {
        VK_TRACE(vkFreeCommandBuffers(
            m_device.device(), m_device.graphicsCommandPool(), 1, &m_handle));
    }
}

void VKCommandBuffer::begin(BeginFlags flags) {
    VkCommandBufferBeginInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.flags = static_cast<VkCommandBufferUsageFlags>(flags);
    VK_TRACE(vkBeginCommandBuffer(m_handle, &info));
}

void VKCommandBuffer::record(const DeviceCommand& cmd) {
    translateVulkanCommand(cmd);
}

void VKCommandBuffer::record(const std::span<const DeviceCommand>& cmds) {
    for (const auto& cmd : cmds) record(cmd);
}

void VKCommandBuffer::end() { VK_TRACE(vkEndCommandBuffer(m_handle)); }

VkCommandBuffer VKCommandBuffer::handle() { return m_handle; }

VKWorkload::VKWorkload(VKDevice& device)
    : m_cmdBuffer(std::make_unique<VKCommandBuffer>(device)),
      m_fence(std::make_shared<VKFence>(device)),
      m_semaphore(std::make_shared<VKSemaphore>(device)) {}

void VKWorkload::addDependency(std::shared_ptr<VKSemaphore> semaphore) {
    m_waitSemaphores.push_back(std::move(semaphore));
}

VKCommandBuffer& VKWorkload::commandBuffer() { return *m_cmdBuffer; }

std::shared_ptr<VKFence> VKWorkload::fence() { return m_fence; }

std::shared_ptr<VKSemaphore> VKWorkload::semaphore() { return m_semaphore; }

std::span<std::shared_ptr<VKSemaphore>> VKWorkload::waitSemaphores() {
    return m_waitSemaphores;
}

}  // namespace ignis
