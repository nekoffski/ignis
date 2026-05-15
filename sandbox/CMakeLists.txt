add_executable(rhi rhi.cpp)
add_executable(renderer renderer.cpp)
add_executable(renderpass renderpass.cpp)
add_executable(shader shader.cpp)

target_link_libraries(rhi PRIVATE ignis)
target_link_libraries(renderer PRIVATE ignis)
target_link_libraries(renderpass PRIVATE ignis)
target_link_libraries(shader PRIVATE ignis)