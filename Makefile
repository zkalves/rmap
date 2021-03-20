.PHONY: all run test check clean rebuild

# Default target: compile using existing build files
all: build/Makefile
	@cmake --build build

# Only run CMake configuration if build/Makefile doesn't exist
build/Makefile: CMakeLists.txt
	@mkdir -p build
	@cd build && cmake ..

# Build only application binary
rmap: build/Makefile
	@cmake --build build --target rmap

# Run executable (builds rmap binary only)
run: rmap
	@./build/bin/rmap

# Run automated test suites (cleans up output directory first)
test check: all
	@rm -rf work
	@mkdir -p work
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

# Clean up build directory and test artifacts
clean:
	@rm -rf build work

rebuild: clean all
