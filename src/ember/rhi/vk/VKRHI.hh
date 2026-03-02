#pragma once

#include <memory>

#include "VK.hh"
#include "ember/core/Concepts.hh"
#include "ember/rhi/RHI.hh"
#include "ember/rhi/Window.hh"

namespace ember {

class VKRHI : public RHI {
   public:
    explicit VKRHI(Window* window);
    virtual ~VKRHI() override;

    static std::unique_ptr<VKRHI> create(Window* window);

   private:
    VK m_vk;
};

}  // namespace ember
