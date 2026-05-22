.PHONY: all build build-release build-debug run clean fmt shaders

DXC := LD_LIBRARY_PATH=tools/dxc/lib tools/dxc/bin/dxc
SHADER_DIR := sandbox/res

all: build

build: build-release

build-release:
	conan install . --build=missing -s build_type=Release
	cmake --preset conan-release
	cmake --build --preset conan-release

build-debug:
	conan install . --build=missing -s build_type=Debug
	cmake --preset conan-debug
	cmake --build --preset conan-debug

run: build-release
	./build/Release/sandbox/sandbox

fmt:
	find src sandbox -name '*.cpp' -o -name '*.hh' -o -name '*.h' | xargs clang-format -i

shaders:
	$(DXC) -spirv -T vs_6_0 -E main $(SHADER_DIR)/triangle.vert.hlsl -Fo $(SHADER_DIR)/triangle.vert.spv
	$(DXC) -spirv -T ps_6_0 -E main $(SHADER_DIR)/triangle.frag.hlsl -Fo $(SHADER_DIR)/triangle.frag.spv

clean:
	rm -rf build CMakeUserPresets.json
