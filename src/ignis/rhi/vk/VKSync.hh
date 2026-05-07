#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Time.hh"

namespace ignis::rhi {

class VKDevice;

class VKFence : public NonCopyable, public NonMovable {
   public:
    enum class State {
        signaled,
        unsignaled,
    };

    explicit VKFence(VKDevice& device, State initialState = State::unsignaled);
    ~VKFence();

    void reset();
    bool wait(
        std::chrono::nanoseconds timeout = std::chrono::nanoseconds::max()
    );

    State state() const;
    VkFence handle();

   private:
    VKDevice& m_device;
    VkFence m_handle{VK_NULL_HANDLE};
    State m_state;
};

class VKSemaphore : public NonCopyable, public NonMovable {
   public:
    explicit VKSemaphore(VKDevice& device);
    ~VKSemaphore();

    VkSemaphore handle() const;
    VkSemaphore* handlePtr();

   private:
    VKDevice& m_device;
    VkSemaphore m_handle{VK_NULL_HANDLE};
};

}  // namespace ignis::rhi
