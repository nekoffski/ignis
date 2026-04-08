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

class Device : public NonCopyable, public NonMovable {
   public:
    static std::unique_ptr<Device> create(const Config& config,
                                          Window* window = nullptr);

    virtual ~Device() = default;

    virtual bool headless() const = 0;

    virtual Result<DeviceWorkloadReceipt> submit(
        const DeviceWorkload& workload) = 0;
    virtual OError wait(DeviceWorkloadReceipt receipt) = 0;

    virtual DeviceBufferHandle createBuffer(
        const DeviceBufferDescription& desc) = 0;
    virtual void destroyBuffer(DeviceBufferHandle handle) = 0;

    virtual DeviceTextureHandle createTexture(
        const DeviceTextureMetadata& metadata) = 0;
    virtual void destroyTexture(DeviceTextureHandle handle) = 0;
};

}  // namespace ignis
