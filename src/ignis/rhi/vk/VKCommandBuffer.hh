#pragma once

#include <memory>

#include "VK.hh"
#include "VKSync.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/rhi/DeviceCommand.hh"

namespace ignis {

class VKDevice;

class VKCommandBuffer : public NonCopyable, public NonMovable {
   public:
    enum class BeginFlags : u32 {
        none = 0x00000000,
        oneTimeSubmit = 0x00000001,
        renderPassContinue = 0x00000002,
        simultaneousUse = 0x00000004,
    };

    explicit VKCommandBuffer(VKDevice& device, DeviceQueue targetQueue);
    ~VKCommandBuffer();

    void begin(BeginFlags flags = BeginFlags::none);
    void record(const DeviceCommand& cmd);
    void record(const std::span<const DeviceCommand>& cmds);
    void end();

    template <typename Callable>
        requires std::invocable<Callable>
    void with(Callable&& fn, BeginFlags flags = BeginFlags::none) {
        begin(flags);
        std::invoke(std::forward<Callable>(fn));
        end();
    }

    VkCommandBuffer handle();

   private:
    VKDevice& m_device;
    DeviceQueue m_targetQueue;
    VkCommandBuffer m_handle{VK_NULL_HANDLE};
};

class VKWorkload : public NonCopyable {
   public:
    explicit VKWorkload(VKDevice& device, DeviceQueue targetQueue);

    void addDependency(std::shared_ptr<VKSemaphore> semaphore);

    VKCommandBuffer& commandBuffer();
    std::shared_ptr<VKFence> fence();
    std::shared_ptr<VKSemaphore> semaphore();
    std::span<std::shared_ptr<VKSemaphore>> waitSemaphores();

   private:
    std::unique_ptr<VKCommandBuffer> m_cmdBuffer;
    std::shared_ptr<VKFence> m_fence;
    std::shared_ptr<VKSemaphore> m_semaphore;
    std::vector<std::shared_ptr<VKSemaphore>> m_waitSemaphores;
};

IGNIS_BIT_ENUM(VKCommandBuffer::BeginFlags)

}  // namespace ignis
