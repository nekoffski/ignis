#include "VK.hh"

#include <ranges>
#include <vulkan/vulkan.hpp>

#include "ember/core/Config.hh"
#include "ember/core/Log.hh"

namespace ember {

namespace {
u32 apiVersion() {
    switch (cfg().vulkan().api) {
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

VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
    [[maybe_unused]] void* pUserData) {
    switch (messageSeverity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            log::warn("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;

        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            log::error("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;

        default:
            log::info("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
            break;
    }

    return false;
}

}  // namespace

VK::VK(Window* window) : m_window(window) {
    if (not m_window) log::warn("Bootstraping vulkan without surface support");

    createInstance();
    createDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

void VK::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = cfg().core().appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(
        cfg().version().major, cfg().version().minor, cfg().version().patch);
    appInfo.pEngineName = "Ember";
    appInfo.engineVersion = VK_MAKE_VERSION(
        cfg().version().major, cfg().version().minor, cfg().version().patch);
    appInfo.apiVersion = apiVersion();

    std::vector<const char*> extensions;
    std::transform(cfg().vulkan().extensions.begin(),
                   cfg().vulkan().extensions.end(),
                   std::back_inserter(extensions),
                   [](const std::string& ext) { return ext.c_str(); });
    assertExtensions(extensions);

    std::vector<const char*> layers;
    std::transform(cfg().vulkan().layers.begin(), cfg().vulkan().layers.end(),
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
}

void VK::createDebugMessenger() {
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

void VK::pickPhysicalDevice() {}

void VK::createLogicalDevice() {}

const VkAllocationCallbacks* VK::allocator() const { return nullptr; }

VK::~VK() {
    if (m_device != VK_NULL_HANDLE) vkDestroyDevice(m_device, allocator());

    if (m_debugMessenger != VK_NULL_HANDLE) {
        static const auto debugDestructorFunctionName =
            "vkDestroyDebugUtilsMessengerEXT";

        auto destroyDebugMessenger =
            reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(m_instance, debugDestructorFunctionName));
        destroyDebugMessenger(m_instance, m_debugMessenger, allocator());
    }

    if (m_instance != VK_NULL_HANDLE)
        vkDestroyInstance(m_instance, allocator());
}

}  // namespace ember
