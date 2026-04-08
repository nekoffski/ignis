#include "VKDevice.hh"

#include "VKBootstrap.hh"
#include "VKQueue.hh"
#include "ignis/core/Profiler.hh"

namespace ignis {

static constexpr u32 maxPendingWorkloads = 16u;

VKDevice::VKDevice(const Config& config, Window* window)
    : m_cfg(config), m_window(window), m_pendingWorkloads(maxPendingWorkloads) {
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

Result<DeviceWorkloadReceipt> VKDevice::submit(const DeviceWorkload& wl) {
    log::expect(m_queues.contains(wl.targetQueue()),
                "Workload target queue is not valid");

    auto slot = m_pendingWorkloads.create(*this);

    if (not slot) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to submit workload: pending workload pool is full");
    }

    auto* workload = m_pendingWorkloads.get(*slot);

    for (const auto& dep : wl.dependencies()) {
        if (auto* depWorkload = m_pendingWorkloads.get(dep); depWorkload) {
            workload->addDependency(depWorkload->semaphore());
        } else {
            log::error(
                "Failed to find dependency for workload submission: invalid "
                "receipt");
        }
    }

    workload->commandBuffer().with(
        [&]() { workload->commandBuffer().record(wl.commands()); });

    auto q = m_queues.at(wl.targetQueue());

    if (not VKQueueSubmitter{q, *workload}.submit()) {
        log::error("Failed to submit workload to queue");
        m_pendingWorkloads.destroy(*slot);
        return Error::unexpected(Error::Code::queueSubmissionFailed,
                                 "Failed to submit workload to queue");
    }
    return slot.value();
}

OError VKDevice::wait(DeviceWorkloadReceipt receipt) {
    auto* workload = m_pendingWorkloads.get(receipt);

    if (not workload)
        return Error{Error::Code::resourceMissing, "Invalid workload receipt"};

    auto success = workload->fence()->wait();

    if (not success) {
        log::error("Failed to wait for workload completion");
        return Error{Error::Code::queueSubmissionFailed,
                     "Failed to wait for workload completion"};
    }

    m_pendingWorkloads.destroy(receipt);
    return Error::empty();
}

bool VKDevice::headless() const { return m_window == nullptr; }

VkInstance VKDevice::instance() const { return m_instance; }

VkPhysicalDevice VKDevice::physicalDevice() const { return m_physicalDevice; }

VkDevice VKDevice::device() const { return m_device; }

Allocator VKDevice::allocator() const { return m_allocator; }

VKDeviceInfo VKDevice::deviceInfo() const { return m_deviceInfo; }

VkCommandPool VKDevice::graphicsCommandPool() const {
    return m_graphicsCommandPool;
}

DeviceBufferHandle VKDevice::createBuffer(const DeviceBufferDescription& desc) {

}

void VKDevice::destroyBuffer(DeviceBufferHandle handle) {}

DeviceTextureHandle VKDevice::createTexture(
    const DeviceTextureMetadata& metadata) {}

void VKDevice::destroyTexture(DeviceTextureHandle handle) {}

}  // namespace ignis
