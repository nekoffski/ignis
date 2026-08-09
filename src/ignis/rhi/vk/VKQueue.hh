#pragma once

#include <span>
#include <unordered_map>

#include "VK.hh"
#include "VKCommandBuffer.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis::rhi {

using VKQueueSet = std::unordered_map<Queue, VkQueue>;

struct VKTimelineWait {
    VkSemaphore semaphore{VK_NULL_HANDLE};
    u64 value{0};
};

class VKQueueSubmitter : public NonCopyable, public NonMovable {
   public:
    explicit VKQueueSubmitter(
        VkQueue queue, VKCommandBuffer& commandBuffer,
        std::span<const VKTimelineWait> waits, VkSemaphore signalSemaphore,
        u64 signalValue
    );

    bool submit();

   private:
    VkQueue m_queue;
    VKCommandBuffer& m_commandBuffer;
    std::span<const VKTimelineWait> m_waits;
    VkSemaphore m_signalSemaphore;
    u64 m_signalValue;
};

}  // namespace ignis::rhi
