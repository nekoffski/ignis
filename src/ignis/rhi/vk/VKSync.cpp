#include "VKSync.hh"

#include "VKDevice.hh"

namespace ignis::rhi {

namespace {

void logError(VkResult result) {
    switch (result) {
        case VK_TIMEOUT:
            log::warn("Timeline semaphore wait timed out");
            break;
        case VK_ERROR_DEVICE_LOST:
            log::error("Timeline semaphore wait failed: device lost");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            log::error("Timeline semaphore wait failed: out of host memory");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            log::error("Timeline semaphore wait failed: out of device memory");
            break;
        default:
            log::error("Timeline semaphore wait failed: {}", toString(result));
            break;
    }
}

}  // namespace

VKTimelineSemaphore::VKTimelineSemaphore(VKDevice& device) : m_device(device) {
    VkSemaphoreTypeCreateInfo timelineInfo{};
    timelineInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    timelineInfo.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;
    timelineInfo.initialValue = 0;

    VkSemaphoreCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    info.pNext = &timelineInfo;
    VK_TRACE(vkCreateSemaphore(
        m_device.device(), &info, m_device.allocator(), &m_handle
    ));
}

VKTimelineSemaphore::~VKTimelineSemaphore() {
    if (m_handle != VK_NULL_HANDLE) {
        VK_TRACE(vkDestroySemaphore(
            m_device.device(), m_handle, m_device.allocator()
        ));
    }
}

bool VKTimelineSemaphore::wait(u64 value, std::chrono::nanoseconds timeout) {
    VkSemaphoreWaitInfo waitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &m_handle;
    waitInfo.pValues = &value;

    const auto result = vkWaitSemaphores(
        m_device.device(), &waitInfo, static_cast<u64>(timeout.count())
    );
    if (result == VK_SUCCESS) {
        return true;
    }
    logError(result);
    return false;
}

u64 VKTimelineSemaphore::value() const {
    u64 currentValue = 0;
    VK_TRACE(
        vkGetSemaphoreCounterValue(m_device.device(), m_handle, &currentValue)
    );
    return currentValue;
}

VkSemaphore VKTimelineSemaphore::handle() const { return m_handle; }

}  // namespace ignis::rhi
