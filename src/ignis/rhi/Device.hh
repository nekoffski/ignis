#pragma once

#include <memory>

#include "Buffer.hh"
#include "Texture.hh"
#include "Window.hh"
#include "Workload.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

class Device : public NonCopyable, public NonMovable {
    friend class BufferProxy;

   public:
    static std::unique_ptr<Device> create(const Config& config,
                                          Window* window = nullptr);

    virtual ~Device() = default;

    virtual bool headless() const = 0;

    virtual Result<WorkloadReceipt> submit(const Workload& workload) = 0;
    virtual Opt<Error> wait(WorkloadReceipt receipt) = 0;

    virtual Result<BufferHandle> createBuffer(
        const BufferDescription& desc) = 0;
    virtual void destroyBuffer(BufferHandle handle) = 0;

    virtual Result<TextureHandle> createTexture(
        const TextureDefinition& metadata) = 0;
    virtual void destroyTexture(TextureHandle handle) = 0;

   private:
    virtual BufferProxy::Impl* proxy(BufferHandle handle) = 0;
};

}  // namespace ignis::rhi
