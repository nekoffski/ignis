#include "VKBootstrap.hh"

#include "ignis/core/Profiler.hh"
#include "vulkan/vulkan.hpp"

namespace ignis::rhi {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*
) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            log::warn("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            log::error("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;
        default:
            break;
    }
    return false;
}

u32 apiVersion(const Config& cfg) {
    switch (cfg.vulkan().api) {
        case Config::Vulkan::Api::v1_3:
            return VK_API_VERSION_1_3;
        default:
            log::panic("Unknown Vulkan API version");
    }
}

void assertExtensions(const std::vector<const char*>& extensions) {
    auto availableExtensions = vk::enumerateInstanceExtensionProperties();
    std::vector<std::string> availableExtensionNames;
    for (const auto& ext : availableExtensions) {
        availableExtensionNames.push_back(ext.extensionName);
    }
    for (const auto& ext : extensions) {
        if (std::ranges::find(availableExtensionNames, ext) ==
            availableExtensionNames.end()) {
            log::panic("Required Vulkan extension {} is not available", ext);
        }
    }
}

std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {
    u32 deviceCount = 0;
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));
    log::expect(deviceCount > 0, "Could not find any physical device");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    VK_ASSERT(vkEnumeratePhysicalDevices(
        instance, &deviceCount, physicalDevices.data()
    ));
    return physicalDevices;
}

std::string deviceTypeName(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return "Discrete GPU";
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return "Integrated GPU";
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return "Virtual GPU";
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return "CPU";
        default:
            return "Other";
    }
}

DeviceType deviceType(VkPhysicalDeviceType type) {
    switch (type) {
        case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
            return DeviceType::integrated;
        case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
            return DeviceType::discrete;
        case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
            return DeviceType::virtualGpu;
        case VK_PHYSICAL_DEVICE_TYPE_CPU:
            return DeviceType::cpu;
        default:
            return DeviceType::other;
    }
}

DeviceCapabilities makeCapabilities(const VKDeviceInfo& info) {
    const auto& properties = info.coreProperties;
    const auto& limits = properties.limits;
    const auto& features12 = info.vulkan12Features;
    const auto& features13 = info.vulkan13Features;

    DeviceCapabilities capabilities{
        .deviceName = properties.deviceName,
        .driverName = info.driverProperties.driverName,
        .driverInfo = info.driverProperties.driverInfo,
        .deviceType = deviceType(properties.deviceType),
        .apiVersion =
            {
                .major = VK_API_VERSION_MAJOR(properties.apiVersion),
                .minor = VK_API_VERSION_MINOR(properties.apiVersion),
                .patch = VK_API_VERSION_PATCH(properties.apiVersion),
            },
        .vendorId = properties.vendorID,
        .deviceId = properties.deviceID,
        .driverVersion = properties.driverVersion,
        .queues =
            {
                .graphics = info.queueIndices.contains(Queue::graphics),
                .compute = info.queueIndices.contains(Queue::compute),
                .transfer = info.queueIndices.contains(Queue::transfer),
                .present = info.queueIndices.contains(Queue::present),
            },
        .features =
            {
                .samplerAnisotropy =
                    info.features.samplerAnisotropy != VK_FALSE,
                .timelineSemaphores = features12.timelineSemaphore != VK_FALSE,
                .synchronization2 = features13.synchronization2 != VK_FALSE,
                .dynamicRendering = features13.dynamicRendering != VK_FALSE,
                .descriptorIndexing = features12.descriptorIndexing != VK_FALSE,
                .runtimeDescriptorArrays =
                    features12.runtimeDescriptorArray != VK_FALSE,
                .partiallyBoundDescriptors =
                    features12.descriptorBindingPartiallyBound != VK_FALSE,
                .variableDescriptorCount =
                    features12.descriptorBindingVariableDescriptorCount !=
                    VK_FALSE,
                .sampledImageNonUniformIndexing =
                    features12.shaderSampledImageArrayNonUniformIndexing !=
                    VK_FALSE,
                .bufferDeviceAddress =
                    features12.bufferDeviceAddress != VK_FALSE,
                .drawIndirectCount = features12.drawIndirectCount != VK_FALSE,
            },
        .limits = {
            .maxImageDimension2D = limits.maxImageDimension2D,
            .maxBoundDescriptorSets = limits.maxBoundDescriptorSets,
            .maxPerStageSampledImages =
                limits.maxPerStageDescriptorSampledImages,
            .maxDescriptorSetSampledImages =
                limits.maxDescriptorSetSampledImages,
            .maxDrawIndirectCount = limits.maxDrawIndirectCount,
            .minUniformBufferOffsetAlignment =
                limits.minUniformBufferOffsetAlignment,
            .timestampPeriodNanoseconds = limits.timestampPeriod,
        },
    };

    capabilities.memoryHeaps.reserve(info.memoryProperties.memoryHeapCount);
    for (u32 index = 0; index < info.memoryProperties.memoryHeapCount;
         ++index) {
        const auto& heap = info.memoryProperties.memoryHeaps[index];
        capabilities.memoryHeaps.push_back({
            .size = heap.size,
            .deviceLocal = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0,
        });
    }
    return capabilities;
}

void showDeviceInfo(const VKDeviceInfo& info) {
    const auto& p = info.coreProperties;

    u32 apiMajor = VK_API_VERSION_MAJOR(p.apiVersion);
    u32 apiMinor = VK_API_VERSION_MINOR(p.apiVersion);
    u32 apiPatch = VK_API_VERSION_PATCH(p.apiVersion);
    u32 drvMajor = VK_API_VERSION_MAJOR(p.driverVersion);
    u32 drvMinor = VK_API_VERSION_MINOR(p.driverVersion);
    u32 drvPatch = VK_API_VERSION_PATCH(p.driverVersion);

    log::debug("------- Physical Device -------");
    log::debug("  Name:           {}", p.deviceName);
    log::debug("  Type:           {}", deviceTypeName(p.deviceType));
    log::debug("  Vendor ID:      0x{:04X}", p.vendorID);
    log::debug("  Device ID:      0x{:04X}", p.deviceID);
    log::debug("  API version:    {}.{}.{}", apiMajor, apiMinor, apiPatch);
    log::debug("  Driver version: {}.{}.{}", drvMajor, drvMinor, drvPatch);
    log::debug("  Driver:         {}", info.capabilities.driverName);
    log::debug(
        "  Queues:         graphics={}, compute={}, transfer={}, present={}",
        info.capabilities.queues.graphics, info.capabilities.queues.compute,
        info.capabilities.queues.transfer, info.capabilities.queues.present
    );
    log::debug(
        "  Baseline:       timeline={}, sync2={}, dynamicRendering={}",
        info.capabilities.features.timelineSemaphores,
        info.capabilities.features.synchronization2,
        info.capabilities.features.dynamicRendering
    );
    log::debug(
        "  Optional:       descriptorIndexing={}, bufferDeviceAddress={}, "
        "drawIndirectCount={}",
        info.capabilities.features.descriptorIndexing,
        info.capabilities.features.bufferDeviceAddress,
        info.capabilities.features.drawIndirectCount
    );

    log::debug("  Memory heaps:");
    const auto& mem = info.memoryProperties;
    for (u32 i = 0; i < mem.memoryHeapCount; ++i) {
        const auto& heap = mem.memoryHeaps[i];
        bool deviceLocal = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;
        log::debug(
            "    Heap[{}]: {:.0f} MiB ({})", i,
            static_cast<float>(heap.size) / (1024.f * 1024.f),
            deviceLocal ? "device-local" : "host"
        );
    }
    log::debug("-------------------------------");
}

}  // namespace

VKBootstrap::VKBootstrap(const Config& config, Window* window)
    : m_cfg(config), m_window(window) {
    if (not window) {
        log::warn("Bootstraping vulkan without surface support");
    }
    IGNIS_PROFILE_FUNCTION();

    createInstance();
    createDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createCommandPools();
    fetchQueues();
}

VkInstance VKBootstrap::instance() const { return m_instance; }

VkPhysicalDevice VKBootstrap::physicalDevice() const {
    return m_physicalDevice;
}

VkDevice VKBootstrap::device() const { return m_device; }

Allocator VKBootstrap::allocator() const { return m_allocator; }

VkDebugUtilsMessengerEXT VKBootstrap::debugMessenger() const {
    return m_debugMessenger;
}

const VKDeviceInfo& VKBootstrap::deviceInfo() const { return m_deviceInfo; }

void VKBootstrap::createInstance() {
    IGNIS_PROFILE_FUNCTION();

    const auto& [major, minor, patch] = m_cfg.version();

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_cfg.core().appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(major, minor, patch);
    appInfo.pEngineName = "Ignis";
    appInfo.engineVersion = VK_MAKE_VERSION(major, minor, patch);
    appInfo.apiVersion = apiVersion(m_cfg);

    std::vector<const char*> extensions;
    std::transform(
        m_cfg.vulkan().extensions.begin(), m_cfg.vulkan().extensions.end(),
        std::back_inserter(extensions),
        [](const std::string& ext) { return ext.c_str(); }
    );
    assertExtensions(extensions);

    std::vector<const char*> layers;
    std::transform(
        m_cfg.vulkan().layers.begin(), m_cfg.vulkan().layers.end(),
        std::back_inserter(layers),
        [](const std::string& layer) { return layer.c_str(); }
    );

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    createInfo.enabledLayerCount = static_cast<u32>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    VK_ASSERT(vkCreateInstance(&createInfo, allocator(), &m_instance));
    log::debug("Vulkan instance created successfully");
}

void VKBootstrap::createDebugMessenger() {
    IGNIS_PROFILE_FUNCTION();

    static const auto debugFactoryFunctionName =
        "vkCreateDebugUtilsMessengerEXT";

    u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                      VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity = logSeverity;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugMessengerCallback;

    auto createDebugMessenger =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, debugFactoryFunctionName)
        );
    log::expect(
        createDebugMessenger, "Failed to create debug messenger factory"
    );
    VK_ASSERT(createDebugMessenger(
        m_instance, &debugCreateInfo, allocator(), &m_debugMessenger
    ));
}

std::pair<std::unordered_map<Queue, u32>, Queue> discoverQueues(
    VkPhysicalDevice physicalDevice
) {
    u32 queueFamilyCount = 0;
    VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, 0
    ));

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, queueFamilies.data()
    ));

    Queue foundQueues = Queue::none;
    std::unordered_map<Queue, u32> indices;

    const auto markIndex = [&](Queue type, u32 index) {
        indices[type] = index;
        foundQueues |= type;
    };

    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFlags = queueFamilies[i].queueFlags;
        if (queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            markIndex(Queue::graphics, i);
        }
        if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
            markIndex(Queue::compute, i);
        }
        if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
            markIndex(Queue::transfer, i);
        }

        // TODO:
        // VkBool32 supportsPresent = false;
        // logExpect(vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
        // surface,
        // &supportsPresent));

        // if (supportsPresent) markIndex(Queue::present, i);
    }
    return {indices, foundQueues};
}

std::optional<VKDeviceInfo> VKBootstrap::DeviceRequirements::fulfills(
    const VkPhysicalDevice& device
) const {
    IGNIS_PROFILE_FUNCTION();
    VKDeviceInfo info{};

    VkPhysicalDeviceDriverProperties driverProperties{};
    driverProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
    VkPhysicalDeviceProperties2 properties{};
    properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    properties.pNext = &driverProperties;
    VK_TRACE(vkGetPhysicalDeviceProperties2(device, &properties));
    info.coreProperties = properties.properties;
    info.driverProperties = driverProperties;
    info.driverProperties.pNext = nullptr;

    VkPhysicalDeviceMemoryProperties2 memoryProperties{};
    memoryProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    VK_TRACE(vkGetPhysicalDeviceMemoryProperties2(device, &memoryProperties));
    info.memoryProperties = memoryProperties.memoryProperties;

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = &features13;
    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &features12;
    VK_TRACE(vkGetPhysicalDeviceFeatures2(device, &features));
    info.features = features.features;
    info.vulkan12Features = features12;
    info.vulkan12Features.pNext = nullptr;
    info.vulkan13Features = features13;
    info.vulkan13Features.pNext = nullptr;

    if (isDiscrete && info.coreProperties.deviceType !=
                          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        return {};
    }

    const bool supportsVulkan13 =
        VK_API_VERSION_MAJOR(info.coreProperties.apiVersion) > 1 ||
        (VK_API_VERSION_MAJOR(info.coreProperties.apiVersion) == 1 &&
         VK_API_VERSION_MINOR(info.coreProperties.apiVersion) >= 3);
    const bool supportsBaseline = supportsVulkan13 &&
                                  info.features.samplerAnisotropy &&
                                  info.vulkan12Features.timelineSemaphore &&
                                  info.vulkan13Features.synchronization2 &&
                                  info.vulkan13Features.dynamicRendering;
    if (not supportsBaseline) {
        log::debug(
            "Device '{}' does not support the Vulkan 1.3 renderer baseline",
            info.coreProperties.deviceName
        );
        return {};
    }

    auto [indices, foundQueues] = discoverQueues(device);

    if ((foundQueues & queues) != queues) {
        log::debug(
            "Device '{}' does not support required queues, required: {}, "
            "found: "
            "{}",
            info.coreProperties.deviceName, static_cast<u32>(queues),
            static_cast<u32>(foundQueues)
        );
        return {};
    }

    info.queueIndices = std::move(indices);
    info.capabilities = makeCapabilities(info);

    // swapchain?
    // if (supportSurface) {
    // }
    return info;
}

void VKBootstrap::pickPhysicalDevice() {
    DeviceRequirements req;
    req.supportSurface = false;
    req.isDiscrete = m_cfg.vulkan().requireDiscreteGPU;
    req.queues = Queue::graphics | Queue::transfer;

    if (m_window) {
        req.supportSurface = true;
        req.queues |= Queue::present;
        req.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    for (const auto& device : getPhysicalDevices(m_instance)) {
        if (const auto info = req.fulfills(device); info) {
            m_physicalDevice = device;
            m_deviceInfo = *info;
            log::debug("Physical device matching requirements found!");
            showDeviceInfo(*info);
            return;
        }
    }
    log::panic("Failed to find a suitable physical device");
}

void VKBootstrap::createLogicalDevice() {
    static constexpr u64 maximumExpectedQueuesCount = 3;

    std::vector<u32> indices;
    indices.reserve(maximumExpectedQueuesCount);
    auto& queueIndices = m_deviceInfo.queueIndices;

    indices.push_back(queueIndices.at(Queue::graphics));

    if (m_window) {
        if (queueIndices.at(Queue::graphics) !=
            queueIndices.at(Queue::present)) {
            indices.push_back(queueIndices.at(Queue::present));
        }
    }

    if (queueIndices.at(Queue::graphics) != queueIndices.at(Queue::transfer)) {
        indices.push_back(queueIndices.at(Queue::transfer));
    }

    if (queueIndices.contains(Queue::compute) &&
        queueIndices.at(Queue::graphics) != queueIndices.at(Queue::compute)) {
        indices.push_back(queueIndices.at(Queue::compute));
    }

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::vector<f32> queueProrities;

    auto queueCount = indices.size();
    queueProrities.reserve(queueCount);
    queueCreateInfos.reserve(queueCount);

    for (const auto index : indices) {
        log::trace("Adding queue family index: {}", index);

        queueProrities.push_back(1.0f);
        VkDeviceQueueCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.queueFamilyIndex = index;
        info.queueCount = 1;
        info.pQueuePriorities = &queueProrities.back();
        queueCreateInfos.push_back(info);
    }

    VkPhysicalDeviceVulkan13Features features13{};
    features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    features13.synchronization2 = VK_TRUE;
    features13.dynamicRendering = VK_TRUE;

    VkPhysicalDeviceVulkan12Features features12{};
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = &features13;
    features12.timelineSemaphore = VK_TRUE;

    VkPhysicalDeviceFeatures2 features{};
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &features12;
    features.features.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> extensionNames;

    if (m_window) {
        extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = &features;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.enabledExtensionCount = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    VK_ASSERT(vkCreateDevice(
        m_physicalDevice, &deviceCreateInfo, m_allocator, &m_device
    ));
    log::trace("vkCreateDevice: {}", static_cast<void*>(m_device));
}

void VKBootstrap::createCommandPools() {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolCreateInfo.queueFamilyIndex =
        m_deviceInfo.queueIndices.at(Queue::graphics);
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_ASSERT(vkCreateCommandPool(
        m_device, &poolCreateInfo, m_allocator, &m_commandPools.graphics
    ));
    log::trace(
        "vkCreateCommandPool: {}", static_cast<void*>(m_commandPools.graphics)
    );

    poolCreateInfo.queueFamilyIndex =
        m_deviceInfo.queueIndices.at(Queue::transfer);
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_ASSERT(vkCreateCommandPool(
        m_device, &poolCreateInfo, m_allocator, &m_commandPools.transfer
    ));
    log::trace(
        "vkCreateCommandPool: {}", static_cast<void*>(m_commandPools.transfer)
    );
}

void VKBootstrap::fetchQueues() {
    for (const auto& [type, index] : m_deviceInfo.queueIndices) {
        VkQueue queue;
        vkGetDeviceQueue(m_device, index, 0, &queue);
        m_queues.emplace(type, queue);
    }
}

VKCommandPools& VKBootstrap::commandPools() { return m_commandPools; }

VKQueueSet VKBootstrap::queues() const { return m_queues; }

}  // namespace ignis::rhi
