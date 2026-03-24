add_library(ignis SHARED
    "Engine.cpp"
    # -- core
    "core/Config.cpp"
    "core/String.cpp"
    "core/Log.cpp"
    "core/String.cpp"
    "core/Error.cpp"
    "core/FileSystem.cpp"
    # -- renderer
    "renderer/Renderer.cpp"
    # -- rhi
    "rhi/Device.cpp"
    "rhi/Window.cpp"
    "rhi/ResourceRegistry.cpp"
    # -- rhi/vk
    "rhi/vk/VulkanDevice.cpp"
    "rhi/vk/VulkanResourceRegistry.cpp"
    "rhi/vk/VulkanBootstrap.cpp"
    "rhi/vk/VulkanQueue.cpp"
)

target_include_directories(ignis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ignis
    PUBLIC
        fmt::fmt
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        vulkan-headers::vulkan-headers
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
)

set_target_properties(ignis PROPERTIES POSITION_INDEPENDENT_CODE ON)
