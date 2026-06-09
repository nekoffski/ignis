SET (SANDBOX_BINARIES
    rhi
    renderer
    renderpass
    shader
    mesh
)

foreach (binary ${SANDBOX_BINARIES})
    add_executable(${binary} ${binary}.cpp)
    target_link_libraries(${binary} PRIVATE ignis)
endforeach ()
