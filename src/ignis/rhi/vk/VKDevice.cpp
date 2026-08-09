#include "VKDevice.hh"

#include <algorithm>

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
    m_capabilities = m_deviceInfo.capabilities;
    m_queues = bootstrap.queues();

    for (const auto& [queue, _] : m_queues) {
        m_timelines.emplace(
            queue, std::make_unique<VKTimelineSemaphore>(*this)
        );
        m_nextTimelineValues.emplace(queue, 0);
        m_lastSubmittedTimelineValues.emplace(queue, 0);
    }
}

VKDevice::~VKDevice() {
    if (not m_device.empty()) {
        VK_TRACE(vkDeviceWaitIdle(*m_device));
        collectCompletedWorkloads();
    }
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

    collectCompletedWorkloads();

    std::vector<VKTimelineWait> waits;
    waits.reserve(wl.dependencies().size());
    for (const auto dependency : wl.dependencies()) {
        if (const auto error = validateTimelinePoint(dependency); error) {
            return Error::unexpected(*error);
        }
        waits.push_back({
            .semaphore = m_timelines.at(dependency.queue)->handle(),
            .value = dependency.value,
        });
    }

    const auto completion = reserveTimelinePoint(wl.targetQueue());
    auto slot = m_pendingWorkloads.create(*this, wl.targetQueue(), completion);

    if (not slot) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to submit workload: pending workload pool is full"
        );
    }

    auto* workload = m_pendingWorkloads.get(*slot);

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
    auto signalSemaphore = m_timelines.at(wl.targetQueue())->handle();

    if (not VKQueueSubmitter{
            q, workload->commandBuffer(), waits, signalSemaphore,
            completion.value
        }
                .submit()) {
        log::error("Failed to submit workload to queue");
        m_pendingWorkloads.destroy(*slot);
        return Error::unexpected(
            Error::Code::queueSubmissionFailed,
            "Failed to submit workload to queue"
        );
    }

    m_nextTimelineValues[completion.queue] = completion.value;
    m_lastSubmittedTimelineValues[completion.queue] = completion.value;
    m_pendingWorkloadKeys.push_back({
        .key = *slot,
        .completion = completion,
    });
    return completion;
}

Opt<Error> VKDevice::wait(WorkloadReceipt receipt) {
    if (const auto error = validateTimelinePoint(receipt); error) {
        return error;
    }

    auto success = m_timelines.at(receipt.queue)->wait(receipt.value);

    if (not success) {
        log::error("Failed to wait for workload completion");
        return Error{
            Error::Code::queueSubmissionFailed,
            "Failed to wait for workload completion"
        };
    }

    collectCompletedWorkloads();
    return Error::empty();
}

TimelinePoint VKDevice::reserveTimelinePoint(Queue queue) {
    return TimelinePoint{
        .queue = queue,
        .value = m_nextTimelineValues.at(queue) + 1,
    };
}

Opt<Error> VKDevice::validateTimelinePoint(TimelinePoint point) const {
    const auto timeline = m_timelines.find(point.queue);
    const auto lastSubmitted = m_lastSubmittedTimelineValues.find(point.queue);
    if (timeline == m_timelines.end() ||
        lastSubmitted == m_lastSubmittedTimelineValues.end() ||
        point.value == 0 || point.value > lastSubmitted->second) {
        return Error{
            Error::Code::resourceMissing,
            "Invalid timeline point for queue '{}': {}", toString(point.queue),
            point.value
        };
    }
    return Error::empty();
}

void VKDevice::collectCompletedWorkloads() {
    std::unordered_map<Queue, u64> completedValues;
    for (const auto& [queue, timeline] : m_timelines) {
        completedValues.emplace(queue, timeline->value());
    }

    std::erase_if(m_pendingWorkloadKeys, [&](const PendingWorkload& pending) {
        if (completedValues.at(pending.completion.queue) <
            pending.completion.value) {
            return false;
        }
        m_pendingWorkloads.destroy(pending.key);
        return true;
    });
}

bool VKDevice::headless() const { return m_window == nullptr; }

Format VKDevice::depthFormat() const {
    return fromVk(m_deviceInfo.depthFormat);
}

const DeviceCapabilities& VKDevice::capabilities() const {
    return m_capabilities;
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
        bool isSuitable =
            (typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags &
                                        vkMemoryProperty) == vkMemoryProperty;
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
