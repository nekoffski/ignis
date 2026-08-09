#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Time.hh"

namespace ignis::rhi {

class VKDevice;

class VKTimelineSemaphore : public NonCopyable, public NonMovable {
   public:
    explicit VKTimelineSemaphore(VKDevice& device);
    ~VKTimelineSemaphore();

    bool wait(
        u64 value,
        std::chrono::nanoseconds timeout = std::chrono::nanoseconds::max()
    );
    u64 value() const;

    VkSemaphore handle() const;

   private:
    VKDevice& m_device;
    VkSemaphore m_handle{VK_NULL_HANDLE};
};

}  // namespace ignis::rhi
