#include "VKQueue.hh"

namespace ignis::rhi {

VKQueueSubmitter::VKQueueSubmitter(
    VkQueue queue, VKCommandBuffer& commandBuffer,
    std::span<const VKTimelineWait> waits, VkSemaphore signalSemaphore,
    u64 signalValue
)
    : m_queue(queue),
      m_commandBuffer(commandBuffer),
      m_waits(waits),
      m_signalSemaphore(signalSemaphore),
      m_signalValue(signalValue) {}

bool VKQueueSubmitter::submit() {
    std::vector<VkSemaphoreSubmitInfo> waitInfos;
    waitInfos.reserve(m_waits.size());
    for (const auto& wait : m_waits) {
        VkSemaphoreSubmitInfo waitInfo{};
        waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        waitInfo.semaphore = wait.semaphore;
        waitInfo.value = wait.value;
        waitInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        waitInfos.push_back(waitInfo);
    }

    VkCommandBufferSubmitInfo commandBufferInfo{};
    commandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferInfo.commandBuffer = m_commandBuffer.handle();

    VkSemaphoreSubmitInfo signalInfo{};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalInfo.semaphore = m_signalSemaphore;
    signalInfo.value = m_signalValue;
    signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

    VkSubmitInfo2 submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.waitSemaphoreInfoCount = static_cast<u32>(waitInfos.size());
    submitInfo.pWaitSemaphoreInfos = waitInfos.data();
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalInfo;

    const auto result = vkQueueSubmit2(m_queue, 1, &submitInfo, VK_NULL_HANDLE);

    if (result != VK_SUCCESS) {
        log::error("Failed to submit workload to queue: {}", toString(result));
        return false;
    }

    return true;
}

}  // namespace ignis::rhi
