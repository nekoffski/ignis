// #include "VK.hh"

// #include <ranges>
// #include <vulkan/vulkan.hpp>

// #include "ignis/core/Config.hh"
// #include "ignis/core/Enum.hh"
// #include "ignis/core/Log.hh"
// #include "ignis/core/ServiceLocator.hh"

// namespace ignis {

// namespace {
// u32 apiVersion() {
//     const auto& cfg = ServiceLocator<Config>::get();

//     switch (cfg.vulkan().api) {
//         case Config::Vulkan::Api::v1_3:
//             return VK_API_VERSION_1_3;
//         default:
//             log::panic("Unknown Vulkan API version");
//     }
// }

// std::vector<VkPhysicalDevice> getPhysicalDevices(VkInstance instance) {
//     u32 deviceCount = 0;
//     VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount, 0));
//     log::expect(deviceCount > 0, "Could not find any physical device");

//     std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
//     VK_ASSERT(vkEnumeratePhysicalDevices(instance, &deviceCount,
//                                          physicalDevices.data()));

//     return physicalDevices;
// }

// void assertExtensions(const std::vector<const char*>& extensions) {
//     auto availableExtensions = vk::enumerateInstanceExtensionProperties();
//     std::vector<std::string> availableExtensionNames;
//     for (const auto& ext : availableExtensions)
//         availableExtensionNames.push_back(ext.extensionName);

//     for (const auto& ext : extensions) {
//         if (std::ranges::find(availableExtensionNames, ext) ==
//             availableExtensionNames.end()) {
//             log::panic("Required Vulkan extension {} is not available", ext);
//         }
//     }
// }

// VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(
//     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//     [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
//     VkDebugUtilsMessengerCallbackDataEXT const* pCallbackData,
//     [[maybe_unused]] void* pUserData) {
//     switch (messageSeverity) {
//         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
//             log::warn("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
//             break;

//         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
//             log::error("VK_DEBUG_LAYER - {}", pCallbackData->pMessage);
//             break;

//         default:
//             break;
//     }
//     return false;
// }

// void showDeviceInfo(const VkPhysicalDevice& device) {}

// }  // namespace

// VK::VK(Window* window) : m_window(window) {
//     if (not m_window) log::warn("Bootstraping vulkan without surface
//     support");

//     createInstance();
//     createDebugMessenger();
//     pickPhysicalDevice();
//     createLogicalDevice();
// }

// void VK::createInstance() {
//     const auto& cfg = ServiceLocator<Config>::get();

//     VkApplicationInfo appInfo{};
//     appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//     appInfo.pApplicationName = cfg.core().appName.c_str();
//     appInfo.applicationVersion = VK_MAKE_VERSION(
//         cfg.version().major, cfg.version().minor, cfg.version().patch);
//     appInfo.pEngineName = "Ignis";
//     appInfo.engineVersion = VK_MAKE_VERSION(
//         cfg.version().major, cfg.version().minor, cfg.version().patch);
//     appInfo.apiVersion = apiVersion();

//     std::vector<const char*> extensions;
//     std::transform(cfg.vulkan().extensions.begin(),
//                    cfg.vulkan().extensions.end(),
//                    std::back_inserter(extensions),
//                    [](const std::string& ext) { return ext.c_str(); });
//     assertExtensions(extensions);

//     std::vector<const char*> layers;
//     std::transform(cfg.vulkan().layers.begin(), cfg.vulkan().layers.end(),
//                    std::back_inserter(layers),
//                    [](const std::string& layer) { return layer.c_str(); });

//     VkInstanceCreateInfo createInfo{};
//     createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//     createInfo.pApplicationInfo = &appInfo;
//     createInfo.enabledExtensionCount = static_cast<u32>(extensions.size());
//     createInfo.ppEnabledExtensionNames = extensions.data();
//     createInfo.enabledLayerCount = static_cast<u32>(layers.size());
//     createInfo.ppEnabledLayerNames = layers.data();

//     VK_ASSERT(vkCreateInstance(&createInfo, allocator(), &m_instance));
//     log::debug("Vulkan instance created successfully");
// }

// void VK::createDebugMessenger() {
//     static const auto debugFactoryFunctionName =
//         "vkCreateDebugUtilsMessengerEXT";

//     u32 logSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
//                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
//                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
//     VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

//     debugCreateInfo.sType =
//         VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//     debugCreateInfo.messageSeverity = logSeverity;
//     debugCreateInfo.messageType =
//         VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
//         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
//     debugCreateInfo.pfnUserCallback = debugMessengerCallback;

//     auto createDebugMessenger =
//         reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
//             vkGetInstanceProcAddr(m_instance, debugFactoryFunctionName));
//     log::expect(createDebugMessenger,
//                 "Failed to create debug messenger factory");
//     VK_ASSERT(createDebugMessenger(m_instance, &debugCreateInfo, allocator(),
//                                    &m_debugMessenger));
// }

// std::pair<std::unordered_map<Queue::Type, u32>, Queue::Type>
// VK::assignQueues(
//     const VK::DeviceRequirements& req) const {
//     u32 queueFamilyCount = 0;
//     VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice,
//                                                       &queueFamilyCount, 0));

//     std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
//     VK_TRACE(vkGetPhysicalDeviceQueueFamilyProperties(
//         m_physicalDevice, &queueFamilyCount, queueFamilies.data()));

//     Queue::Type foundQueues = Queue::Type::none;
//     std::unordered_map<Queue::Type, u32> indices;

//     const auto markIndex = [&](Queue::Type type, u32 index) {
//         indices[type] = index;
//         foundQueues |= type;
//     };

//     for (u32 i = 0; i < queueFamilyCount; ++i) {
//         const auto& queueFlags = queueFamilies[i].queueFlags;
//         if (queueFlags & VK_QUEUE_GRAPHICS_BIT)
//             markIndex(Queue::Type::graphics, i);
//         if (queueFlags & VK_QUEUE_COMPUTE_BIT)
//             markIndex(Queue::Type::compute, i);
//         if (queueFlags & VK_QUEUE_TRANSFER_BIT)
//             markIndex(Queue::Type::transfer, i);

//         // TODO:
//         // VkBool32 supportsPresent = false;
//         // log::vkExpect(vkGetPhysicalDeviceSurfaceSupportKHR(device, i,
//         // surface,
//         // &supportsPresent));

//         // if (supportsPresent) markIndex(Queue::Type::present, i);
//     }
//     return {indices, foundQueues};
// }

// std::optional<VK::DeviceInfo> VK::DeviceRequirements::fulfills(
//     const VkPhysicalDevice& device) const {
//     VK::DeviceInfo info;

//     VK_TRACE(vkGetPhysicalDeviceProperties(device, &info.coreProperties));
//     VK_TRACE(
//         vkGetPhysicalDeviceMemoryProperties(device, &info.memoryProperties));
//     VK_TRACE(vkGetPhysicalDeviceFeatures(device, &info.features));

//     if (isDiscrete && info.coreProperties.deviceType !=
//                           VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
//         return {};
//     }

//     // queues
//     // todo: at this point the devices is not assigned yet, so method does
//     not
//     // make sesnse either devicePicker {} or make free functions
//     // const auto [indices, foundQueues] = assignQueues(req);

//     // // swapchain?
//     // if (supportSurface) {
//     // }

//     // return info;
// }

// void VK::pickPhysicalDevice() {
//     DeviceRequirements req;
//     req.supportSurface = false;
//     req.isDiscrete = true;
//     req.queues = Queue::Type::graphics | Queue::Type::transfer;

//     if (not headless()) {
//         req.supportSurface = true;
//         req.queues |= Queue::Type::present;
//         req.extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
//     }

//     for (const auto& device : getPhysicalDevices(m_instance)) {
//         if (const auto info = req.fulfills(device); info) {
//             m_physicalDevice = device;
//             m_deviceInfo = *info;
//             log::debug("Physical device matching requirements found!");
//             showDeviceInfo(device);
//             return;
//         }
//     }
//     log::panic("Failed to find a suitable physical device");
// }

// void VK::createLogicalDevice() {}

// const VkAllocationCallbacks* VK::allocator() const { return nullptr; }

// VK::~VK() {
//     if (m_device != VK_NULL_HANDLE)
//         VK_TRACE(vkDestroyDevice(m_device, allocator()));

//     if (m_debugMessenger != VK_NULL_HANDLE) {
//         static const auto debugDestructorFunctionName =
//             "vkDestroyDebugUtilsMessengerEXT";

//         auto destroyDebugMessenger =
//             reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
//                 vkGetInstanceProcAddr(m_instance,
//                 debugDestructorFunctionName));
//         destroyDebugMessenger(m_instance, m_debugMessenger, allocator());
//     }

//     if (m_instance != VK_NULL_HANDLE)
//         VK_TRACE(vkDestroyInstance(m_instance, allocator()));
// }

// bool VK::headless() const { return m_window == nullptr; }

// const VK::DeviceInfo& VK::deviceInfo() const { return m_deviceInfo; }

// }  // namespace ignis
