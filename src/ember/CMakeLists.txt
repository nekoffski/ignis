add_library(ember_lib
    core/Core.cpp
)

target_include_directories(ember_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(ember_lib
    PUBLIC
        fmt::fmt
)
