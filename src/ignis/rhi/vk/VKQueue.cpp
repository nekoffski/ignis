#include "VKQueue.hh"

namespace ignis {

VKQueueSubmitter::VKQueueSubmitter(VkQueue q, VKWorkload& workload)
    : m_q(q), m_workload(workload) {}

bool VKQueueSubmitter::submit() {
    VkSubmitInfo submitInfo{};

    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    auto& commandBuffer = m_workload.commandBuffer();
    auto waitSemaphores = m_workload.waitSemaphores();

    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = m_workload.semaphore()->handlePtr();

    std::vector<VkSemaphore> waitSemaphoreHandles;
    for (const auto& semaphore : waitSemaphores)
        waitSemaphoreHandles.push_back(semaphore->handle());

    submitInfo.waitSemaphoreCount =
        static_cast<u32>(waitSemaphoreHandles.size());
    submitInfo.pWaitSemaphores = waitSemaphoreHandles.data();

    std::vector<VkCommandBuffer> commandBufferHandles{commandBuffer.handle()};
    submitInfo.commandBufferCount =
        static_cast<u32>(commandBufferHandles.size());
    submitInfo.pCommandBuffers = commandBufferHandles.data();

    VkPipelineStageFlags flags[1] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.pWaitDstStageMask = flags;

    const auto result =
        vkQueueSubmit(m_q, 1, &submitInfo, m_workload.fence()->handle());

    if (result != VK_SUCCESS) {
        log::error("Failed to submit workload to queue: {}", toString(result));
        return false;
    }

    return true;
}

}  // namespace ignis
