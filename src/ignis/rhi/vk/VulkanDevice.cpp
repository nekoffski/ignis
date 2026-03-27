#include "VulkanDevice.hh"

#include "VulkanBootstrap.hh"

namespace ignis {

VulkanDevice::VulkanDevice(const Config& config, Window* window)
    : m_cfg(config), m_window(window) {
    VulkanBootstrap bootstrap{config, window};

    m_instance = bootstrap.instance();
    m_allocator = bootstrap.allocator();
    m_debugMessenger = bootstrap.debugMessenger();
    m_physicalDevice = bootstrap.physicalDevice();
    m_device = bootstrap.device();
    m_deviceInfo = bootstrap.deviceInfo();
    m_graphicsCommandPool = bootstrap.graphicsCommandPool();
    m_queues = bootstrap.queues();
}

VulkanDevice::~VulkanDevice() {
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

Device::WorkloadReceipt VulkanDevice::submit(const Workload&) { return 0u; }

void VulkanDevice::wait(WorkloadReceipt receipt) {}

bool VulkanDevice::headless() const { return m_window == nullptr; }

VkInstance VulkanDevice::instance() const { return m_instance; }

VkPhysicalDevice VulkanDevice::physicalDevice() const {
    return m_physicalDevice;
}

VkDevice VulkanDevice::device() const { return m_device; }

Allocator VulkanDevice::allocator() const { return m_allocator; }

VulkanDeviceInfo VulkanDevice::deviceInfo() const { return m_deviceInfo; }

}  // namespace ignis
