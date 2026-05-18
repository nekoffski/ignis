# Ignis — Project Guidelines

Ignis is an early-stage, experimental C++23 renderer engine with a Vulkan backend. Expect many systems to be stubs or WIP. Prefer incremental, targeted changes over refactors.

> **Maintainers:** keep this file up to date as the project evolves. When a convention changes, a new layer is introduced, a dependency is added/removed, or the status of a subsystem changes, update the relevant section here before or alongside the code change.

## Build & Run

```bash
make build-debug      # Conan install + cmake --preset conan-debug + build
make build-release    # Conan install + cmake --preset conan-release + build
make run              # Run the sandbox executable
make fmt              # Format code (clang-format)
make clean            # Remove build artifacts
```

Manual (if needed):

```bash
conan install . --build=missing -s build_type=Debug
cmake --preset conan-debug && cmake --build --preset conan-debug
```

Build outputs: `build/Debug/` and `build/Release/`. Conan-generated CMake files live in `build/*/generators/`.

## Architecture

Four layers (only interact with the layer directly below you):

```
Application / Sandbox   →   sandbox/
Engine                  →   src/ignis/Engine.hh
Renderer                →   src/ignis/renderer/
RHI (abstract)          →   src/ignis/rhi/
  └─ Vulkan impl        →   src/ignis/rhi/vk/      ← Vulkan is ONLY here
```

**Vulkan must not leak** past `rhi/vk/`. Public headers in `rhi/` expose handle types and enums only — never `VkXxx` types.

## File & Naming Conventions

- Headers use `.hh` (not `.hpp`)
- Private members: `m_camelCase`
- Free functions and methods: `camelCase()`
- Classes: `PascalCase`
- Vulkan implementation files: `VK` prefix (e.g., `VKDevice.hh`)

## Type Aliases (from `core/Core.hh`)

Always use these instead of raw C++ types:

```cpp
u8, u16, u32, u64    // unsigned integers
i8, i16, i32, i64    // signed integers
f32, f64             // float/double
Str                  // std::string
```

## Error Handling

No exceptions. Use:

- `Result<T>` (`std::expected<T, Error>`) for fallible functions that return a value
- `Opt<Error>` (`std::optional<Error>`) for fallible functions with no return value
- `log::panic(...)` for unrecoverable states (aborts)
- `log::expect(cond, ...)` for assertion-style checks

```cpp
Result<Buffer> create(...);       // may fail, returns value
Opt<Error>         uploadData(...);         // may fail, no value
```

## Resource Handles

GPU resources are accessed via typed generational handles (`DeviceBufferHandle`, `DeviceTextureHandle`). Never hold raw Vulkan handles or raw pointers to GPU resources outside `rhi/vk/`.

## Bit-Flag Enums

Use the `IGNIS_BIT_ENUM(EnumName)` macro to enable bitwise operators on flag enums:

```cpp
IGNIS_BIT_ENUM(DeviceBufferUsage)
auto usage = DeviceBufferUsage::Vertex | DeviceBufferUsage::Index;
bool isVertex = checkFlag(usage, DeviceBufferUsage::Vertex);
```

## Logging

```cpp
log::info("loaded {} textures", count);
log::debug("...");
log::warn("...");
log::error("...");
log::panic("unrecoverable: {}", reason);   // [[noreturn]]
```

Source location is injected automatically — do not pass `__FILE__`/`__LINE__`.

## Class Defaults

Most classes inherit `NonCopyable` and/or `NonMovable` from `core/Concepts.hh`. Apply this to any class owning GPU resources or holding unique state. Do not add copy constructors/operators to such classes.

## Platform & Backend

- **Current:** Linux, Vulkan only
- **Planned:** macOS, Windows; additional backends possible
- Do not hard-code platform paths or Vulkan-only assumptions in layers above `rhi/vk/`

## Key Dependencies

| Library                  | Purpose                                  |
| ------------------------ | ---------------------------------------- |
| `fmt` / `spdlog`         | Formatting and logging                   |
| `glm`                    | 3D math (via `core/Math.hh`)             |
| `vulkan-headers` + `VMA` | Vulkan API + memory allocation           |
| `spirv-reflect`          | SPIR-V reflection (shader introspection) |
| `fastgltf`               | glTF 2.0 parsing (asset pipeline, WIP)   |
| `stb`                    | Image read/write                         |
| `tomlplusplus`           | Config parsing                           |

## Status

Much of the **renderer layer** and **scene/ECS** is stubbed. Prefer extending existing skeletons over introducing new abstractions. The **core** and **RHI foundation** are the most stable areas.

## Command / Workload Pattern

GPU work is recorded into a `Workload`, submitted to `Device::submit()`, and awaited with `Device::wait(receipt)`.

```cpp
Workload wl{Queue::graphics};
wl.enqueue(CmdBeginRenderPass{...});
wl.enqueue(CmdEndRenderPass{...});
auto receipt = device.submit(wl);
device.wait(*receipt);
```

**Internals (two-pass dispatch inside `VKCommandDispatcher`):**

1. `preprocessCommands()` — collects all referenced handles into `VKCommandManifest`
2. `VKCommandContext::consume()` — resolves handles to live pointers (fails fast if any are invalid)
3. `recordCommands()` — emits Vulkan API calls via `recordCommand()` overloads

**Key types:**

| Type                  | Location          | Purpose                                                |
| --------------------- | ----------------- | ------------------------------------------------------ |
| `Workload`            | `rhi/Workload.hh` | User-facing command accumulator, bound to one `Queue`  |
| `WorkloadReceipt`     | `rhi/Workload.hh` | Opaque token (`u8`) for GPU wait / dependency chaining |
| `Command`             | `rhi/Command.hh`  | `std::variant` of all command structs                  |
| `VKCommandDispatcher` | `rhi/vk/`         | Two-pass preprocess → record engine                    |
| `VKCommandManifest`   | `rhi/vk/`         | Pre-flight handle set (validate before recording)      |
| `VKCommandContext`    | `rhi/vk/`         | Resolved pointer map used during recording             |

**Adding a new command:**

1. Add a struct + `CommandType` entry in `rhi/Command.hh`, add to the `Command` variant
2. Add `preprocessCommand(VKCommandManifest&, const NewCmd&)` in the dispatcher
3. Add `recordCommand(const VKCommandContext&, const NewCmd&)` in the dispatcher

**Existing commands:** `CmdBeginRenderPass`, `CmdEndRenderPass`, `CmdUploadBufferToTexture`, `CmdDownloadTextureToBuffer`. Draw, bind-shader, bind-descriptor-set, and push-constant commands are **not yet implemented**.
