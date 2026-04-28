#pragma once

#include "ResourceHandle.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"
#include "ignis/core/Error.hh"

namespace ignis::rhi {

using BufferHandle = ResourceHandle<ResourceType::buffer>;

enum class BufferUsage : u64 {
    undefined = 0x0,
    transferSrc = 0x00000001,
    transferDst = 0x00000002,
    uniformTexelBuffer = 0x00000004,
    storageTexelBuffer = 0x00000008,
    uniformBuffer = 0x00000010,
    storageBuffer = 0x00000020,
    indexBuffer = 0x00000040,
    vertexBuffer = 0x00000080,
    indirectBuffer = 0x00000100,
    shaderDeviceAddress = 0x00020000,
    videoDecodeSrcBitKhr = 0x00002000,
    videoDecodeDstBitKhr = 0x00004000,
    transformFeedbackBufferBitExt = 0x00000800,
    transformFeedbackCounterBufferBitExt = 0x00001000,
    conditionalRenderingBitExt = 0x00000200,
    accelerationStructureStorageBitKhr = 0x00100000,
    shaderBindingTableBitKhr = 0x00000400,
    videoEncodeDstBitKhr = 0x00008000,
    videoEncodeSrcBitKhr = 0x00010000,
    samplerDescriptorBufferBitExt = 0x00200000,
    resourceDescriptorBufferBitExt = 0x00400000,
    pushDescriptorsDescriptorBufferBitExt = 0x04000000,
    micromapBuildInputReadOnlyBitExt = 0x00800000,
    micromapStorageBitExt = 0x01000000,
    flagBitsMaxEnum = 0x7FFFFFFF
};

enum class MemoryProperty : u64 {
    undefined = 0x0,
    deviceLocal = 0x00000001,
    hostVisible = 0x00000002,
    hostCoherent = 0x00000004,
    hostCached = 0x00000008,
    lazilyAllocated = 0x00000010,
    _protected = 0x00000020,
    deviceCoherentBitAmd = 0x00000040,
    deviceUncachedBitAmd = 0x00000080,
    rdmaCapableBitNv = 0x00000100,
};

struct BufferDescription {
    u64 size;
    BufferUsage usage;
    MemoryProperty memoryProperty;
    bool bindOnCreation;

    static BufferDescription staging(u64 size);
};

class Device;

class BufferProxy {
   public:
    class Impl : public virtual NonCopyable {
       public:
        virtual ~Impl() = default;

        virtual void write(const void* data, const Range& range) = 0;
        virtual void read(void* data, const Range& range) = 0;
    };

    Opt<Error> write(const void* data, const Range& range);
    Opt<Error> read(void* data, const Range& range);

    explicit BufferProxy(Device& device, BufferHandle handle);

   private:
    Device& m_device;
    BufferHandle m_handle;
};

IGNIS_BIT_ENUM(BufferUsage);
IGNIS_BIT_ENUM(MemoryProperty);

}  // namespace ignis::rhi
