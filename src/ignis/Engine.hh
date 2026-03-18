#pragma once

#include "ignis/core/Concepts.hh"
#include "ignis/core/Config.hh"
#include "ignis/core/Core.hh"
#include "ignis/renderer/Renderer.hh"

namespace ignis {

class Engine : public NonCopyable, public NonMovable {
   public:
    explicit Engine(const Config& config);

    Renderer& renderer();

   private:
    Config m_config;
    Renderer m_renderer;
};

}  // namespace ignis
