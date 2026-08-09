#pragma once

#include <memory>

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/rhi/Command.hh"

namespace ignis::rhi {

class VKDevice;

class VKCommandBuffer : public NonCopyable, public NonMovable {
   public:
    enum class BeginFlags : u32 {
        none = 0x00000000,
        oneTimeSubmit = 0x00000001,
        renderPassContinue = 0x00000002,
        simultaneousUse = 0x00000004,
    };

    explicit VKCommandBuffer(VKDevice& device, Queue targetQueue);
    ~VKCommandBuffer();

    void begin(BeginFlags flags = BeginFlags::none);
    void end();

    template <typename Callback>
        requires Callable<Callback, Opt<Error>(VkCommandBuffer, Queue)>
    Opt<Error> with(Callback&& fn, BeginFlags flags = BeginFlags::none) {
        begin(flags);

        auto err =
            std::invoke(std::forward<Callback>(fn), m_handle, m_targetQueue);

        if (err) {
            log::error("Failed to record command buffer: {}", err->message());
            return err;
        }

        end();
        return Error::empty();
    }

    VkCommandBuffer handle();

   private:
    VKDevice& m_device;
    Queue m_targetQueue;
    VkCommandBuffer m_handle{VK_NULL_HANDLE};
};

class VKWorkload : public NonCopyable {
   public:
    explicit VKWorkload(
        VKDevice& device, Queue targetQueue, TimelinePoint completion
    );

    VKCommandBuffer& commandBuffer();
    TimelinePoint completion() const;

   private:
    std::unique_ptr<VKCommandBuffer> m_cmdBuffer;
    TimelinePoint m_completion;
};

IGNIS_BIT_ENUM(VKCommandBuffer::BeginFlags)

}  // namespace ignis::rhi
