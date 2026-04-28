#pragma once

#include <unordered_map>

#include "VK.hh"
#include "VKCommandBuffer.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/Queue.hh"

namespace ignis::rhi {

using VKQueueSet = std::unordered_map<Queue, VkQueue>;

class VKQueueSubmitter : public NonCopyable, public NonMovable {
   public:
    explicit VKQueueSubmitter(VkQueue q, VKWorkload& workload);

    bool submit();

   private:
    VkQueue m_q;
    VKWorkload& m_workload;
};

}  // namespace ignis::rhi
