#pragma once

#include <vector>

#include "ignis/core/Core.hh"

namespace ignis::rhi {

enum class DeviceType { other, integrated, discrete, virtualGpu, cpu };

struct ApiVersion {
    u32 major{0};
    u32 minor{0};
    u32 patch{0};
};

struct DeviceCapabilities {
    struct QueueSupport {
        bool graphics{false};
        bool compute{false};
        bool transfer{false};
        bool present{false};
    };

    struct FeatureSupport {
        bool samplerAnisotropy{false};
        bool timelineSemaphores{false};
        bool synchronization2{false};
        bool dynamicRendering{false};
        bool descriptorIndexing{false};
        bool runtimeDescriptorArrays{false};
        bool partiallyBoundDescriptors{false};
        bool variableDescriptorCount{false};
        bool sampledImageNonUniformIndexing{false};
        bool bufferDeviceAddress{false};
        bool drawIndirectCount{false};
    };

    struct Limits {
        u32 maxImageDimension2D{0};
        u32 maxBoundDescriptorSets{0};
        u32 maxPerStageSampledImages{0};
        u32 maxDescriptorSetSampledImages{0};
        u32 maxDrawIndirectCount{0};
        u64 minUniformBufferOffsetAlignment{0};
        f32 timestampPeriodNanoseconds{0};
    };

    struct MemoryHeap {
        u64 size{0};
        bool deviceLocal{false};
    };

    Str deviceName;
    Str driverName;
    Str driverInfo;
    DeviceType deviceType{DeviceType::other};
    ApiVersion apiVersion;
    u32 vendorId{0};
    u32 deviceId{0};
    u32 driverVersion{0};
    QueueSupport queues;
    FeatureSupport features;
    Limits limits;
    std::vector<MemoryHeap> memoryHeaps;
};

}  // namespace ignis::rhi
