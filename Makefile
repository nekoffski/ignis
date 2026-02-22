.PHONY: all build build-release build-debug run clean

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

clean:
	rm -rf build CMakeUserPresets.json
