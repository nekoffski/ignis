#include "VKDevice.hh"

#include "VKBootstrap.hh"
#include "VKCommandDispatcher.hh"
#include "VKQueue.hh"
#include "ignis/core/Profiler.hh"

namespace ignis::rhi {

static constexpr u32 maxPendingWorkloads = 16u;

VKDevice::VKDevice(const Config& config, Window* window)
    : m_cfg(config),
      m_window(window),
      m_pendingWorkloads(maxPendingWorkloads),
      m_resourceManager(*this, config) {
    IGNIS_PROFILE_FUNCTION();

    VKBootstrap bootstrap{config, window};

    m_instance = Scoped(bootstrap.instance(), [](VkInstance& instance) {
        if (instance != VK_NULL_HANDLE) {
            VK_TRACE(vkDestroyInstance(instance, nullptr));
        }
    });

    m_debugMessenger = Scoped(
        bootstrap.debugMessenger(),
        [this](VkDebugUtilsMessengerEXT& messenger) {
            if (messenger != VK_NULL_HANDLE) {
                static const auto debugDestructorFunctionName =
                    "vkDestroyDebugUtilsMessengerEXT";

                auto destroyDebugMessenger =
                    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                        vkGetInstanceProcAddr(
                            *m_instance, debugDestructorFunctionName
                        )
                    );
                destroyDebugMessenger(*m_instance, messenger, m_allocator);
            }
        }
    );

    m_physicalDevice = bootstrap.physicalDevice();

    m_device = Scoped(bootstrap.device(), [this](VkDevice& device) {
        if (device != VK_NULL_HANDLE) {
            VK_TRACE(vkDestroyDevice(device, m_allocator));
        }
    });

    m_allocator = bootstrap.allocator();

    m_commandPools =
        Scoped(bootstrap.commandPools(), [this](VKCommandPools& pools) {
            if (pools.graphics != VK_NULL_HANDLE) {
                VK_TRACE(
                    vkDestroyCommandPool(*m_device, pools.graphics, m_allocator)
                );
            }
            if (pools.transfer != VK_NULL_HANDLE) {
                VK_TRACE(
                    vkDestroyCommandPool(*m_device, pools.transfer, m_allocator)
                );
            }
        });

    m_deviceInfo = bootstrap.deviceInfo();
    m_queues = bootstrap.queues();
}

Result<WorkloadReceipt> VKDevice::submit(const Workload& wl) {
    log::debug(
        "Submitting workload with {} commands to queue '{}'",
        wl.commands().size(), toString(wl.targetQueue())
    );
    log::expect(
        m_queues.contains(wl.targetQueue()),
        "Workload target queue is not valid"
    );

    auto slot = m_pendingWorkloads.create(*this, wl.targetQueue());

    if (not slot) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to submit workload: pending workload pool is full"
        );
    }

    auto* workload = m_pendingWorkloads.get(*slot);

    for (const auto& dep : wl.dependencies()) {
        if (auto* depWorkload = m_pendingWorkloads.get(dep); depWorkload) {
            workload->addDependency(depWorkload->semaphore());
        } else {
            log::error(
                "Failed to find dependency for workload submission: invalid "
                "receipt"
            );
        }
    }

    auto err = workload->commandBuffer().with([&](auto handle, auto queue) {
        return VKCommandDispatcher{m_resourceManager, handle, queue}.dispatch(
            wl.commands()
        );
    });

    if (err) {
        log::error("Failed to record command buffer for workload submission");
        m_pendingWorkloads.destroy(*slot);
        return Error::unexpected(*err);
    }

    auto q = m_queues.at(wl.targetQueue());

    if (not VKQueueSubmitter{q, *workload}.submit()) {
        log::error("Failed to submit workload to queue");
        m_pendingWorkloads.destroy(*slot);
        return Error::unexpected(
            Error::Code::queueSubmissionFailed,
            "Failed to submit workload to queue"
        );
    }
    return slot.value();
}

Opt<Error> VKDevice::wait(WorkloadReceipt receipt) {
    auto* workload = m_pendingWorkloads.get(receipt);

    if (not workload) {
        return Error{Error::Code::resourceMissing, "Invalid workload receipt"};
    }

    auto success = workload->fence()->wait();

    if (not success) {
        log::error("Failed to wait for workload completion");
        return Error{
            Error::Code::queueSubmissionFailed,
            "Failed to wait for workload completion"
        };
    }

    m_pendingWorkloads.destroy(receipt);
    return Error::empty();
}

bool VKDevice::headless() const { return m_window == nullptr; }

Format VKDevice::depthFormat() const {
    return fromVk(m_deviceInfo.depthFormat);
}

VkInstance VKDevice::instance() const { return *m_instance; }

VkPhysicalDevice VKDevice::physicalDevice() const { return m_physicalDevice; }

VkDevice VKDevice::device() const { return *m_device; }

Allocator VKDevice::allocator() const { return m_allocator; }

VKDeviceInfo VKDevice::deviceInfo() const { return m_deviceInfo; }

const VKCommandPools& VKDevice::commandPools() const { return *m_commandPools; }

Opt<i32> VKDevice::findMemoryIndex(
    u32 typeFilter, MemoryProperty memoryProperty
) {
    auto vkMemoryProperty = toVk(memoryProperty);
    const auto& props = m_deviceInfo.memoryProperties;
    for (u32 i = 0; i < props.memoryTypeCount; ++i) {
        bool isSuitable = (typeFilter & (1 << i)) &&
                          (props.memoryTypes[i].propertyFlags & vkMemoryProperty
                          ) == vkMemoryProperty;
        if (isSuitable) {
            return i;
        }
    }
    log::warn(
        "Unable to find suitable memory type: {}/{}", typeFilter,
        vkMemoryProperty
    );
    return {};
}

bool VKDevice::supportsFormat(
    Format format, Tiling tiling, TextureUsage usage
) {
    static constexpr struct {
        TextureUsage usage;
        VkFormatFeatureFlags feature;
    } usageToFeature[] = {
        {TextureUsage::transferSrc, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT},
        {TextureUsage::transferDest, VK_FORMAT_FEATURE_TRANSFER_DST_BIT},
        {TextureUsage::sampled, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT},
        {TextureUsage::storage, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT},
        {TextureUsage::colorAttachment, VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT},
        {TextureUsage::depthStencilAttachment,
         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT},
    };

    VkFormatProperties props{};
    vkGetPhysicalDeviceFormatProperties(m_physicalDevice, toVk(format), &props);

    const VkFormatFeatureFlags available = tiling == Tiling::linear
                                               ? props.linearTilingFeatures
                                               : props.optimalTilingFeatures;

    for (const auto& [u, f] : usageToFeature) {
        if (checkFlag(usage, u) && not(available & f)) {
            return false;
        }
    }
    return true;
}

VkQueue VKDevice::queue(Queue type) const {
    log::expect(m_queues.contains(type), "Requested queue type is not valid");
    return m_queues.at(type);
}

u32 VKDevice::queueIndex(Queue type) const {
    log::expect(
        m_deviceInfo.queueIndices.contains(type),
        "Requested queue type is not valid"
    );
    return m_deviceInfo.queueIndices.at(type);
}

VKResourceManager& VKDevice::resources() { return m_resourceManager; }

}  // namespace ignis::rhi
