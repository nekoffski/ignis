#include "VKSync.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

namespace {

void logError(VkResult result) {
    switch (result) {
        case VK_TIMEOUT:
            log::warn("vk_fence_wait - Timed out");
            break;
        case VK_ERROR_DEVICE_LOST:
            log::error("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            log::error("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            log::error("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
            break;
        default:
            log::error("vk_fence_wait - An unknown error has occurred.");
            break;
    }
}

}  // namespace

VKFence::VKFence(VKDevice& device, State initialState)
    : m_device(device), m_state(initialState) {
    VkFenceCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (initialState == State::signaled)
        info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_TRACE(vkCreateFence(m_device.device(), &info, m_device.allocator(),
                           &m_handle));
}

VKFence::~VKFence() {
    if (m_handle != VK_NULL_HANDLE) {
        VK_TRACE(
            vkDestroyFence(m_device.device(), m_handle, m_device.allocator()));
    }
}

void VKFence::reset() {
    if (m_state == State::unsignaled) return;

    VK_TRACE(vkResetFences(m_device.device(), 1, &m_handle));
    m_state = State::unsignaled;
}

bool VKFence::wait(std::chrono::nanoseconds timeout) {
    if (m_state == State::signaled) return true;

    auto result = vkWaitForFences(m_device.device(), 1, &m_handle, VK_TRUE,
                                  timeout.count());
    if (result == VK_SUCCESS) {
        m_state = State::signaled;
        return true;
    }

    logError(result);
    return false;
}

VKFence::State VKFence::state() const { return m_state; }

VkFence VKFence::handle() { return m_handle; }

VKSemaphore::VKSemaphore(VKDevice& device) : m_device(device) {
    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_TRACE(vkCreateSemaphore(m_device.device(), &info, m_device.allocator(),
                               &m_handle));
}

VKSemaphore::~VKSemaphore() {
    if (m_handle != VK_NULL_HANDLE) {
        VK_TRACE(vkDestroySemaphore(m_device.device(), m_handle,
                                    m_device.allocator()));
    }
}

VkSemaphore VKSemaphore::handle() const { return m_handle; }
VkSemaphore* VKSemaphore::handlePtr() { return &m_handle; }

}  // namespace ignis::rhi
