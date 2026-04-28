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
      m_bufferPool(64u),
      m_texturePool(64u) {
    IGNIS_PROFILE_FUNCTION();

    VKBootstrap bootstrap{config, window};

    m_instance = Scoped(bootstrap.instance(), [](VkInstance& instance) {
        if (instance != VK_NULL_HANDLE)
            VK_TRACE(vkDestroyInstance(instance, nullptr));
    });

    m_debugMessenger = Scoped(
        bootstrap.debugMessenger(),
        [this](VkDebugUtilsMessengerEXT& messenger) {
            if (messenger != VK_NULL_HANDLE) {
                static const auto debugDestructorFunctionName =
                    "vkDestroyDebugUtilsMessengerEXT";

                auto destroyDebugMessenger =
                    reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                        vkGetInstanceProcAddr(*m_instance,
                                              debugDestructorFunctionName));
                destroyDebugMessenger(*m_instance, messenger, m_allocator);
            }
        });

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
                VK_TRACE(vkDestroyCommandPool(*m_device, pools.graphics,
                                              m_allocator));
            }
            if (pools.transfer != VK_NULL_HANDLE) {
                VK_TRACE(vkDestroyCommandPool(*m_device, pools.transfer,
                                              m_allocator));
            }
        });

    m_deviceInfo = bootstrap.deviceInfo();
    m_queues = bootstrap.queues();
}

Result<WorkloadReceipt> VKDevice::submit(const Workload& wl) {
    log::debug("Submitting workload with {} commands to queue '{}'",
               wl.commands().size(), toString(wl.targetQueue()));
    log::expect(m_queues.contains(wl.targetQueue()),
                "Workload target queue is not valid");

    auto slot = m_pendingWorkloads.create(*this, wl.targetQueue());

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

    auto err = workload->commandBuffer().with([&](auto handle, auto q) {
        return VKCommandDispatcher{*this, handle, q}.dispatch(wl.commands());
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
        return Error::unexpected(Error::Code::queueSubmissionFailed,
                                 "Failed to submit workload to queue");
    }
    return slot.value();
}

Opt<Error> VKDevice::wait(WorkloadReceipt receipt) {
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

VkInstance VKDevice::instance() const { return *m_instance; }

VkPhysicalDevice VKDevice::physicalDevice() const { return m_physicalDevice; }

VkDevice VKDevice::device() const { return *m_device; }

Allocator VKDevice::allocator() const { return m_allocator; }

VKDeviceInfo VKDevice::deviceInfo() const { return m_deviceInfo; }

const VKCommandPools& VKDevice::commandPools() const { return *m_commandPools; }

Result<BufferHandle> VKDevice::createBuffer(const BufferDescription& desc) {
    auto id = m_bufferPool.create([&](u32 slot) {
        return ResourceWrapper{
            VKBuffer{
                *this,
                desc,
            },
            BufferHandle{slot},
        };
    });

    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create buffer: buffer pool is full");
    }
    return m_bufferPool.get(*id)->handle;
}

void VKDevice::destroyBuffer(BufferHandle handle) {
    m_bufferPool.destroy(handle.id);
}

Result<TextureHandle> VKDevice::createTexture(
    const TextureDefinition& definition) {
    auto id = m_texturePool.create([&](u32 slot) {
        return ResourceWrapper{
            VKTexture{
                *this,
                definition.image,
                definition.metadata,
                definition.sampler,
            },
            TextureHandle{slot},
        };
    });

    if (not id) {
        return Error::unexpected(
            Error::Code::poolFull,
            "Failed to create texture: texture pool is full");
    }
    return m_texturePool.get(*id)->handle;
}

void VKDevice::destroyTexture(TextureHandle handle) {
    m_texturePool.destroy(handle.id);
}

VKTexture* VKDevice::findTexture(TextureHandle handle) {
    if (auto textureWrapper = m_texturePool.get(handle.id); textureWrapper)
        return &textureWrapper->resource;
    log::warn("Failed to get texture proxy: invalid texture handle: {}",
              static_cast<u32>(handle.id));
    return nullptr;
}

VKBuffer* VKDevice::findBuffer(BufferHandle handle) {
    if (auto bufferWrapper = m_bufferPool.get(handle.id); bufferWrapper)
        return &bufferWrapper->resource;
    log::warn("Failed to get buffer proxy: invalid buffer handle: {}",
              static_cast<u32>(handle.id));
    return nullptr;
}

// Opt<Error> VKDevice::transfer(BufferToTextureTransfer copy,
//                           Workload& workload) {
//     // auto bufferWrapper = m_bufferPool.get(copy.from.id);
//     // if (not bufferWrapper)
//     //     return Error{Error::Code::resourceMissing, "Invalid buffer
//     handle"};

//     // auto textureWrapper = m_texturePool.get(copy.to.id);
//     // if (not textureWrapper)
//     //     return Error{Error::Code::resourceMissing, "Invalid texture
//     handle"};

//     // auto& buffer = bufferWrapper->resource;
//     // auto& texture = textureWrapper->resource;

//     // auto& workload = m_pendingWorkloads.get(workloadReceipt);
//     // if (not workload)

//     // texture.copyFrom(buffer, *workload.);

//     return Error::empty();
// }

Opt<i32> VKDevice::findMemoryIndex(u32 typeFilter,
                                   MemoryProperty memoryProperty) {
    auto vkMemoryProperty = toVk(memoryProperty);
    const auto& props = m_deviceInfo.memoryProperties;
    for (u32 i = 0; i < props.memoryTypeCount; ++i) {
        bool isSuitable =
            (typeFilter & (1 << i)) && (props.memoryTypes[i].propertyFlags &
                                        vkMemoryProperty) == vkMemoryProperty;
        if (isSuitable) return i;
    }
    log::warn("Unable to find suitable memory type: {}/{}", typeFilter,
              vkMemoryProperty);
    return {};
}

bool VKDevice::supportsFormat(TextureFormat format, TextureTiling tiling,
                              TextureUsage usage) {
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

    const VkFormatFeatureFlags available = tiling == TextureTiling::linear
                                               ? props.linearTilingFeatures
                                               : props.optimalTilingFeatures;

    for (const auto& [u, f] : usageToFeature)
        if (checkFlag(usage, u) && not(available & f)) return false;
    return true;
}

VkQueue VKDevice::queue(Queue type) const {
    log::expect(m_queues.contains(type), "Requested queue type is not valid");
    return m_queues.at(type);
}

u32 VKDevice::queueIndex(Queue type) const {
    log::expect(m_deviceInfo.queueIndices.contains(type),
                "Requested queue type is not valid");
    return m_deviceInfo.queueIndices.at(type);
}

BufferProxy::Impl* VKDevice::proxy(BufferHandle handle) {
    if (auto bufferWrapper = m_bufferPool.get(handle.id); bufferWrapper)
        return &bufferWrapper->resource;
    log::error("Failed to get buffer proxy: invalid buffer handle");
    return nullptr;
}

}  // namespace ignis::rhi
