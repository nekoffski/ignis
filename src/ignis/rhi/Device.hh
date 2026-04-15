#pragma once

#include <memory>

#include "DeviceBuffer.hh"
#include "DeviceTexture.hh"
#include "DeviceWorkload.hh"
#include "Window.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"

namespace ignis {

struct HostToBufferTransfer {
    void* from;
    DeviceBufferHandle to;
    u64 size;
};

struct BufferToTextureTransfer {
    DeviceBufferHandle from;
    DeviceTextureHandle to;
};

struct TextureToBufferTransfer {
    DeviceTextureHandle from;
    DeviceBufferHandle to;
};

struct BufferToHostTransfer {
    DeviceBufferHandle from;
    void* to;
    u64 size;
};

class Device : public NonCopyable, public NonMovable {
   public:
    static std::unique_ptr<Device> create(const Config& config,
                                          Window* window = nullptr);

    virtual ~Device() = default;

    virtual bool headless() const = 0;

    virtual Result<DeviceWorkloadReceipt> submit(
        const DeviceWorkload& workload) = 0;
    virtual OError wait(DeviceWorkloadReceipt receipt) = 0;

    virtual Result<DeviceBufferHandle> createBuffer(
        const DeviceBufferDescription& desc) = 0;
    virtual void destroyBuffer(DeviceBufferHandle handle) = 0;

    virtual Result<DeviceTextureHandle> createTexture(
        const DeviceTextureDefinition& metadata) = 0;
    virtual void destroyTexture(DeviceTextureHandle handle) = 0;

    virtual OError transfer(HostToBufferTransfer copy) = 0;

    virtual OError transfer(BufferToTextureTransfer copy) = 0;
    virtual OError transfer(BufferToTextureTransfer copy,
                            DeviceWorkload& workload) = 0;

    virtual OError transfer(TextureToBufferTransfer copy,
                            DeviceWorkload& workload) = 0;
    virtual OError transfer(TextureToBufferTransfer copy) = 0;

    virtual OError transfer(BufferToHostTransfer copy) = 0;
};

}  // namespace ignis
