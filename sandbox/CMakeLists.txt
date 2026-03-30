add_executable(rhi rhi.cpp)
add_executable(renderer renderer.cpp)

target_link_libraries(rhi PRIVATE ignis)
target_link_libraries(renderer PRIVATE ignis)