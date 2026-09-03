.PHONY: all run test check clean rebuild docs docs-serve test-templates

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

# Run comprehensive template verification tests across all 11 output formats
test-templates: rmap
	@rm -rf work/test_templates
	@mkdir -p work/test_templates
	@python3 tests/test_template.py all

# Clean up build directory and test artifacts
clean:
	@rm -rf build work _site

rebuild: clean all

# Render documentation portal locally
docs:
	@./script/render_docs

# Serve documentation locally with Pelican
docs-serve:
	@export PYTHONPATH="$$(pwd)/.python_packages:$${PYTHONPATH}"; \
	if python3 -m pelican --version >/dev/null 2>&1; then \
		echo "Serving documentation on http://127.0.0.1:8000 ..."; \
		python3 -m pelican -l docs -s docs/pelicanconf.py -o _site; \
	elif command -v pelican >/dev/null 2>&1; then \
		echo "Serving documentation on http://127.0.0.1:8000 ..."; \
		pelican -l docs -s docs/pelicanconf.py -o _site; \
	else \
		echo "Pelican not found. Install with: pip install pelican markdown"; \
	fi
