# Developing Ignis

## Configure and build

Install the Conan dependencies and generate the CMake presets before configuring
a fresh build directory.

### Debug

```sh
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug
cmake --build --preset conan-debug
```

### Release

```sh
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

## Tests

Ignis tests use GTest and are discovered individually by CTest.

```sh
ctest --preset conan-debug --output-on-failure
ctest --preset conan-release --output-on-failure
```

Core tests do not require a Vulkan device. Run only those tests with:

```sh
ctest --preset conan-debug --output-on-failure -LE integration
```

Headless RHI tests are labeled `integration` and `vulkan`. They require a Vulkan
1.3 implementation; a software implementation such as lavapipe is sufficient.

```sh
ctest --preset conan-debug --output-on-failure -L integration
```
