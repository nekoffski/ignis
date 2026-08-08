add_executable(ignis_core_tests
    "core/PoolTests.cpp"
)

target_link_libraries(ignis_core_tests PRIVATE ignis GTest::gtest_main)

add_executable(ignis_rhi_tests
    "rhi/HeadlessRhiTests.cpp"
    "rhi/ResourceManagerTests.cpp"
)

target_link_libraries(ignis_rhi_tests PRIVATE ignis GTest::gtest_main)
target_compile_definitions(ignis_rhi_tests PRIVATE
    IGNIS_TEST_CONFIG_PATH="${PROJECT_SOURCE_DIR}/cfg/ignis.toml"
)

include(GoogleTest)
gtest_discover_tests(ignis_core_tests)
gtest_discover_tests(ignis_rhi_tests PROPERTIES LABELS "integration;vulkan")
