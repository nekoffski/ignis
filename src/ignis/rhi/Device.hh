#pragma once

#include <memory>

#include "Window.hh"
#include "Workload.hh"
#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"

namespace ignis {

class Device : public NonCopyable, public NonMovable {
   public:
    using WorkloadReceipt = u8;

    static std::unique_ptr<Device> create(const Config& config,
                                          Window* window = nullptr);

    virtual ~Device() = default;

    virtual bool headless() const = 0;

    virtual WorkloadReceipt submit(const Workload& workload) = 0;
    virtual void wait(WorkloadReceipt receipt) = 0;
};

}  // namespace ignis
