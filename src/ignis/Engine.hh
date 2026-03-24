#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Singleton.hh"
#include "ignis/renderer/Renderer.hh"
#include "ignis/rhi/Device.hh"
#include "ignis/rhi/ResourceRegistry.hh"

namespace ignis {

class Engine : public UniqueInstance<Engine> {
   public:
    explicit Engine(const Config& config);
    ~Engine();

    Renderer& renderer();
    Device& device();
    ResourceRegistry& resourceRegistry();

   private:
    Config m_config;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<ResourceRegistry> m_resourceRegistry;

    Renderer m_renderer;
};

}  // namespace ignis
