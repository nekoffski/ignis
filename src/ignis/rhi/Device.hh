#pragma once

#include <memory>

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
};

}  // namespace ignis
