#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/core/Singleton.hh"
#include "ignis/render/Renderer.hh"
#include "ignis/rhi/Device.hh"

namespace ignis {

class Engine : public UniqueInstance<Engine> {
   public:
    explicit Engine(const Config& config);
    ~Engine();

    render::Renderer& renderer();
    rhi::Device& device();

   private:
    Config m_config;
    std::unique_ptr<rhi::Device> m_device;

    render::Renderer m_renderer;
};

}  // namespace ignis
