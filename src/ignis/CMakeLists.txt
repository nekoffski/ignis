add_library(ignis SHARED
    "Engine.cpp"
    # -- asset
    "asset/Image.cpp"
    "asset/Stb.cpp"
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
    "renderer/RenderGraph.cpp"
    "renderer/RenderGraphLayout.cpp"
    "renderer/RenderScene.cpp"
    "renderer/RenderPass.cpp"
    "renderer/RenderBundle.cpp"
    # -- rhi
    "rhi/Device.cpp"
    "rhi/Window.cpp"
    "rhi/DeviceWorkload.cpp"
    "rhi/DeviceTexture.cpp"
    "rhi/DeviceBuffer.cpp"
    "rhi/DeviceQueue.cpp"
    # -- rhi/vk
    "rhi/vk/VK.cpp"
    "rhi/vk/VKBuffer.cpp"
    "rhi/vk/VKDevice.cpp"
    "rhi/vk/VKBootstrap.cpp"
    "rhi/vk/VKQueue.cpp"
    "rhi/vk/VKCommandBuffer.cpp"
    "rhi/vk/VKCommandDispatcher.cpp"
    "rhi/vk/VKSync.cpp"
    "rhi/vk/VKTexture.cpp"
)

target_include_directories(ignis
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ignis
    PUBLIC
        glm::glm
        fmt::fmt
        spdlog::spdlog
        tomlplusplus::tomlplusplus
        vulkan-headers::vulkan-headers
        Vulkan::Vulkan
        GPUOpen::VulkanMemoryAllocator
        stduuid::stduuid
        stb::stb 
)

set_target_properties(ignis PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(ignis PUBLIC
    $<$<CONFIG:Debug>:IGNIS_ENABLE_PROFILING>
    $<$<CONFIG:Debug>:IGNIS_DEBUG_BUILD>
)
