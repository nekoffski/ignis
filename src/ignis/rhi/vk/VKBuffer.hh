#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Scope.hh"
#include "ignis/rhi/DeviceBuffer.hh"

namespace ignis {

class VKDevice;

class VKBuffer : public NonCopyable {
   public:
    VKBuffer(VKDevice& device, const DeviceBufferDescription& description);
    ~VKBuffer();

    VkBuffer handle() const;
    VkBuffer* handlePtr();

    void copyTo(const void* data, const Range& range);
    void copyFrom(void* data, const Range& range);

    VKBuffer(VKBuffer&&);
    VKBuffer& operator=(VKBuffer&&) = delete;

    void bind(u64 offset = 0);

   private:
    void* lock(const Range& range);
    void unlock();
    void create();

    template <typename Callable>
        requires std::invocable<Callable, void*>
    void with(const Range& range, Callable&& callable) {
        auto ptr = lock(range);
        ON_SCOPE_EXIT { unlock(); };
        callable(ptr);
    }

    VKDevice& m_device;
    DeviceBufferDescription m_description;
    VkBuffer m_handle{VK_NULL_HANDLE};
    VkDeviceMemory m_memory{VK_NULL_HANDLE};
    bool m_locked{false};

    i32 m_memoryIndex{-1};
};

}  // namespace ignis
