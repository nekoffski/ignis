#pragma once

#include <memory>

#include "Buffer.hh"
#include "Pipeline.hh"
#include "RenderPass.hh"
#include "ResourceManager.hh"
#include "Shader.hh"
#include "Texture.hh"
#include "Window.hh"
#include "Workload.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"

namespace ignis::rhi {

class Device : public NonCopyable, public NonMovable {
   public:
    static std::unique_ptr<Device> create(
        const Config& config, Window* window = nullptr
    );

    virtual ~Device() = default;

    virtual bool headless() const = 0;
    virtual Format depthFormat() const = 0;

    virtual Result<WorkloadReceipt> submit(const Workload& workload) = 0;
    virtual Opt<Error> wait(WorkloadReceipt receipt) = 0;

    virtual ResourceManager& resources() = 0;
};

}  // namespace ignis::rhi
