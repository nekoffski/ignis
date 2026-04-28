add_executable(rhi rhi.cpp)
add_executable(renderer renderer.cpp)
add_executable(renderpass renderpass.cpp)

target_link_libraries(rhi PRIVATE ignis)
target_link_libraries(renderer PRIVATE ignis)
target_link_libraries(renderpass PRIVATE ignis)