#pragma once

#include "DeviceResourceHandle.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Enum.hh"

namespace ignis {

using DeviceBufferHandle = DeviceResourceHandle<DeviceResourceType::buffer>;

enum class DeviceBufferUsage : u64 {
    undefined = 0x0,
    transferSrcBit = 0x00000001,
    transferDstBit = 0x00000002,
    uniformTexelBufferBit = 0x00000004,
    storageTexelBufferBit = 0x00000008,
    uniformBufferBit = 0x00000010,
    storageBufferBit = 0x00000020,
    indexBufferBit = 0x00000040,
    vertexBufferBit = 0x00000080,
    indirectBufferBit = 0x00000100,
    shaderDeviceAddressBit = 0x00020000,
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

enum class DeviceMemoryProperty : u64 {
    undefined = 0x0,
    deviceLocalBit = 0x00000001,
    hostVisibleBit = 0x00000002,
    hostCoherentBit = 0x00000004,
    hostCachedBit = 0x00000008,
    lazilyAllocatedBit = 0x00000010,
    protectedBit = 0x00000020,
    deviceCoherentBitAmd = 0x00000040,
    deviceUncachedBitAmd = 0x00000080,
    rdmaCapableBitNv = 0x00000100,
};

struct DeviceBufferDescription {
    u64 size;
    DeviceBufferUsage usage;
    DeviceMemoryProperty memoryProperty;
    bool bindOnCreation;

    static DeviceBufferDescription staging(u64 size);
};

IGNIS_BIT_ENUM(DeviceBufferUsage);
IGNIS_BIT_ENUM(DeviceMemoryProperty);

}  // namespace ignis
