#include "VulkanBootstrap.hh"

#include "ignis/core/Profiler.hh"
#include "vulkan/vulkan.hpp"

namespace ignis {

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData, void*) {
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
    for (const auto& ext : availableExtensions)
        availableExtensionNames.push_back(ext.extensionName);
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
    VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount,
                                         physicalDevices.data()));
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

void showDeviceInfo(const VulkanDeviceInfo& info) {
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

    log::debug("  Memory heaps:");
    const auto& mem = info.memoryProperties;
    for (u32 i = 0; i < mem.memoryHeapCount; ++i) {
        const auto& heap = mem.memoryHeaps[i];
        bool deviceLocal = (heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0;
        log::debug("    Heap[{}]: {:.0f} MiB ({})", i,
                   static_cast<float>(heap.size) / (1024.f * 1024.f),
                   deviceLocal ? "device-local" : "host");
    }
    log::debug("-------------------------------");
}

}  // namespace

VulkanBootstrap::VulkanBootstrap(const Config& config, Window* window)
    : m_cfg(config), m_window(window) {
    if (not window) log::warn("Bootstraping vulkan without surface support");
    IGNIS_PROFILE_FUNCTION();

    createInstance();
    createDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
    createGraphicsCommandPool();
    fetchQueues();
}

VkInstance VulkanBootstrap::instance() const { return m_instance; }

VkPhysicalDevice VulkanBootstrap::physicalDevice() const {
    return m_physicalDevice;
}

VkDevice VulkanBootstrap::device() const { return m_device; }

Allocator VulkanBootstrap::allocator() const { return m_allocator; }

VkDebugUtilsMessengerEXT VulkanBootstrap::debugMessenger() const {
    return m_debugMessenger;
}

const VulkanDeviceInfo& VulkanBootstrap::deviceInfo() const {
    return m_deviceInfo;
}

void VulkanBootstrap::createInstance() {
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
    std::transform(m_cfg.vulkan().extensions.begin(),
                   m_cfg.vulkan().extensions.end(),
                   std::back_inserter(extensions),
                   [](const std::string& ext) { return ext.c_str(); });
    assertExtensions(extensions);

    std::vector<const char*> layers;
    std::transform(m_cfg.vulkan().layers.begin(), m_cfg.vulkan().layers.end(),
                   std::back_inserter(layers),
                   [](const std::string& layer) { return layer.c_str(); });

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

void VulkanBootstrap::createDebugMessenger() {
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
            vkGetInstanceProcAddr(m_instance, debugFactoryFunctionName));
    log::expect(createDebugMessenger,
                "Failed to create debug messenger factory");
    VK_ASSERT(createDebugMessenger(m_instance, &debugCreateInfo, allocator(),
                                   &m_debugMessenger));
}

std::pair<std::unordered_map<Queue, u32>, Queue> discoverQueues(
    VkPhysicalDevice physicalDevice) {
    u32 queueFamilyCount = 0;
    VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
                                                      &queueFamilyCount, 0));

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(
        physicalDevice, &queueFamilyCount, queueFamilies.data()));

    Queue foundQueues = Queue::none;
    std::unordered_map<Queue, u32> indices;

    const auto markIndex = [&](Queue type, u32 index) {
        indices[type] = index;
        foundQueues |= type;
    };

    for (u32 i = 0; i < queueFamilyCount; ++i) {
        const auto& queueFlags = queueFamilies[i].queueFlags;
        if (queueFlags & VK_QUEUE_GRAPHICS_BIT) markIndex(Queue::graphics, i);
        if (queueFlags & VK_QUEUE_COMPUTE_BIT) markIndex(Queue::compute, i);
        if (queueFlags & VK_QUEUE_TRANSFER_BIT) markIndex(Queue::transfer, i);

        // TODO:
        // VkBool32 supportsPresent = false;
        // log::vkExpect(vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
        // surface,
        // &supportsPresent));

        // if (supportsPresent) markIndex(Queue::present, i);
    }
    return {indices, foundQueues};
}

std::optional<VulkanDeviceInfo> VulkanBootstrap::DeviceRequirements::fulfills(
    const VkPhysicalDevice& device) const {
    IGNIS_PROFILE_FUNCTION();
    VulkanDeviceInfo info;

    VK_TRACE(vkGetPhysicalDeviceProperties(device, &info.coreProperties));
    VK_TRACE(
        vkGetPhysicalDeviceMemoryProperties(device, &info.memoryProperties));
    VK_TRACE(vkGetPhysicalDeviceFeatures(device, &info.features));

    if (isDiscrete && info.coreProperties.deviceType !=
                          VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        return {};
    }

    auto [indices, foundQueues] = discoverQueues(device);

    if ((foundQueues & queues) != queues) {
        log::debug(
            "Device '{}' does not support required queues, required: {}, "
            "found: "
            "{}",
            info.coreProperties.deviceName, static_cast<u32>(queues),
            static_cast<u32>(foundQueues));
        return {};
    }

    info.queueIndices = std::move(indices);

    // swapchain?
    // if (supportSurface) {
    // }
    return info;
}

void VulkanBootstrap::pickPhysicalDevice() {
    DeviceRequirements req;
    req.supportSurface = false;
    req.isDiscrete = true;
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

void VulkanBootstrap::createLogicalDevice() {
    static constexpr u64 maximumExpectedQueuesCount = 3;

    std::vector<u32> indices;
    indices.reserve(maximumExpectedQueuesCount);
    auto& queueIndices = m_deviceInfo.queueIndices;

    indices.push_back(queueIndices.at(Queue::graphics));

    if (m_window)
        if (queueIndices.at(Queue::graphics) != queueIndices.at(Queue::present))
            indices.push_back(queueIndices.at(Queue::present));

    if (queueIndices.at(Queue::graphics) != queueIndices.at(Queue::transfer))
        indices.push_back(queueIndices.at(Queue::transfer));

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

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    std::vector<const char*> extensionNames;

    if (m_window) extensionNames.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    VK_ASSERT(vkCreateDevice(m_physicalDevice, &deviceCreateInfo, m_allocator,
                             &m_device));
    log::trace("vkCreateDevice: {}", static_cast<void*>(m_device));
}

void VulkanBootstrap::createGraphicsCommandPool() {
    VkCommandPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    poolCreateInfo.queueFamilyIndex =
        m_deviceInfo.queueIndices.at(Queue::graphics);
    poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_ASSERT(vkCreateCommandPool(m_device, &poolCreateInfo, m_allocator,
                                  &m_graphicsCommandPool));
    log::trace("vkCreateCommandPool: {}",
               static_cast<void*>(m_graphicsCommandPool));
}

void VulkanBootstrap::fetchQueues() {
    for (const auto& [type, index] : m_deviceInfo.queueIndices) {
        VkQueue queue;
        vkGetDeviceQueue(m_device, index, 0, &queue);
        m_queues.emplace(type, queue);
    }
}

VkCommandPool VulkanBootstrap::graphicsCommandPool() const {
    return m_graphicsCommandPool;
}

VulkanQueueSet VulkanBootstrap::queues() const { return m_queues; }

}  // namespace ignis
