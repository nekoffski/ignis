#pragma once

#include "VK.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/rhi/DeviceBuffer.hh"

namespace ignis {

class VKDevice;

class VKBuffer : public NonCopyable, public NonMovable {
   public:
    VKBuffer(VKDevice& device, const DeviceBufferDescription& description);
    ~VKBuffer();

    VkBuffer handle() const;
    VkBuffer* handlePtr();

   private:
    VKDevice& m_device;
    DeviceBufferDescription m_description;
    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    bool m_locked;

    i32 m_memoryIndex;
};

}  // namespace ignis
