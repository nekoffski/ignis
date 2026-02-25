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
make fmt                  # clang-format all src/ and sandbox/ files
```

## Conventions

- No decorative/unnecessary comments in code
- Always show a draft of planned changes and wait for explicit approval before touching any file
- Update CLAUDE.md whenever crucial code/infra is added
- Use TODO.md for storing project tasks and todos
- Run `make fmt` after every code generation
- Run `make build` after every code generation to verify compilation
- Header extension: `.hh` for C++ headers, `.cpp` for sources
