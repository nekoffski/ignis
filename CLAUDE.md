# Ember

C++20 project using Conan + CMake.

## Structure

- `/src/ember/` — library code (target: `ember_lib`), includes as `"ember/..."`
- `/sandbox/` — standalone executable linking to `ember_lib`
- `/conanfile.txt` — Conan dependencies
- `/CMakeLists.txt` — root CMake config
- `/Makefile` — top-level build driver

## Build

```
make / make build         # release build (alias for build-release)
make build-release        # conan + cmake Release
make build-debug          # conan + cmake Debug
make run                  # build-release + run sandbox
make clean                # remove build dir
```

## Conventions

- No decorative/unnecessary comments in code
- Show a quick draft of planned changes and wait for approval before generating code
- Update CLAUDE.md whenever crucial code/infra is added
