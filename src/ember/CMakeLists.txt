add_library(ember SHARED
    "core/Config.cpp"
    "core/String.cpp"
    "core/Log.cpp"
    "core/String.cpp"
    "core/Error.cpp"
    "core/FileSystem.cpp"
    "rhi/RHI.cpp"
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
)
