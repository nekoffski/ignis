#include "VKRHI.hh"

#include "ember/core/Functional.hh"
#include "ember/core/Log.hh"

namespace ember {

std::unique_ptr<VKRHI> VKRHI::create(Window* window) {
    return std::make_unique<VKRHI>(window);
}

VKRHI::VKRHI(Window* window) : m_vk(window) {}

VKRHI::~VKRHI() {}

}  // namespace ember
