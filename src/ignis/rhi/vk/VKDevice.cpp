#include "VKDevice.hh"

#include "VKBootstrap.hh"
#include "ignis/core/Profiler.hh"

namespace ignis {

VKDevice::VKDevice(const Config& config, Window* window)
    : m_cfg(config), m_window(window) {
    IGNIS_PROFILE_FUNCTION();

    VKBootstrap bootstrap{config, window};

    m_instance = bootstrap.instance();
    m_allocator = bootstrap.allocator();
    m_debugMessenger = bootstrap.debugMessenger();
    m_physicalDevice = bootstrap.physicalDevice();
    m_device = bootstrap.device();
    m_deviceInfo = bootstrap.deviceInfo();
    m_graphicsCommandPool = bootstrap.graphicsCommandPool();
    m_queues = bootstrap.queues();
}

VKDevice::~VKDevice() {
    if (m_graphicsCommandPool != VK_NULL_HANDLE) {
        VK_TRACE(
            vkDestroyCommandPool(m_device, m_graphicsCommandPool, m_allocator));
    }

    if (m_device != VK_NULL_HANDLE)
        VK_TRACE(vkDestroyDevice(m_device, m_allocator));

    if (m_debugMessenger != VK_NULL_HANDLE) {
        static const auto debugDestructorFunctionName =
            "vkDestroyDebugUtilsMessengerEXT";

        auto destroyDebugMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, debugDestructorFunctionName));
        destroyDebugMessenger(m_instance, m_debugMessenger, m_allocator);
    }

    if (m_instance != VK_NULL_HANDLE)
        VK_TRACE(vkDestroyInstance(m_instance, m_allocator));
}

Device::WorkloadReceipt VKDevice::submit(const Workload&) { return 0u; }

void VKDevice::wait(WorkloadReceipt receipt) {}

bool VKDevice::headless() const { return m_window == nullptr; }

VkInstance VKDevice::instance() const { return m_instance; }

VkPhysicalDevice VKDevice::physicalDevice() const { return m_physicalDevice; }

VkDevice VKDevice::device() const { return m_device; }

Allocator VKDevice::allocator() const { return m_allocator; }

VKDeviceInfo VKDevice::deviceInfo() const { return m_deviceInfo; }

}  // namespace ignis
