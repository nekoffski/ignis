add_library(ignis SHARED
    "Engine.cpp"
    # -- core
    "core/Config.cpp"
    "core/String.cpp"
    "core/Core.cpp"
    "core/Log.cpp"
    "core/String.cpp"
    "core/Error.cpp"
    "core/FileSystem.cpp"
    "core/Random.cpp"
    "core/Profiler.cpp"
    # -- renderer
    "renderer/Renderer.cpp"
    "renderer/RGraph.cpp"
    "renderer/RGraphLayout.cpp"
    "renderer/RScene.cpp"
    "renderer/RPass.cpp"
    "renderer/RBundle.cpp"
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
        stduuid::stduuid
)

set_target_properties(ignis PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(ignis PUBLIC
    $<$<CONFIG:Debug>:IGNIS_ENABLE_PROFILING>
    $<$<CONFIG:Debug>:IGNIS_DEBUG_BUILD>
)
