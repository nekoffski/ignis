add_library(ember SHARED
    # -- core
    "core/Config.cpp"
    "core/String.cpp"
    "core/Log.cpp"
    "core/String.cpp"
    "core/Error.cpp"
    "core/FileSystem.cpp"
    # -- renderer
    "renderer/Renderer.cpp"
    "renderer/FrameStatistics.cpp"
    "renderer/CommandList.cpp"
    # -- rhi
    "rhi/RHI.cpp"
    "rhi/vk/VK.cpp"
    "rhi/vk/VKRHI.cpp"
)

target_include_directories(ember
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ember
    PUBLIC
        fmt::fmt
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        vulkan-headers::vulkan-headers
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
)

set_target_properties(ember PROPERTIES POSITION_INDEPENDENT_CODE ON)
