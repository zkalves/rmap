# This file is part of rmap.
#
# rmap is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rmap is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rmap.  If not, see <https://www.gnu.org/licenses/>.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

.PHONY: all run test check clean rebuild docs docs-serve test-templates test-unit test-backend test-frontend test-all

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

# Run C++ unit test suites (backend and frontend)
test-unit: all
	@rm -rf work
	@mkdir -p work
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build -L "unit" --output-on-failure

# Run only backend unit tests
test-backend: all
	@rm -rf work
	@mkdir -p work
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build -L "backend" --output-on-failure

# Run only frontend GUI unit tests
test-frontend: all
	@rm -rf work
	@mkdir -p work
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build -L "frontend" --output-on-failure

# Run comprehensive template verification tests across all 15 output templates
test-templates: rmap
	@rm -rf work/test_templates
	@mkdir -p work/test_templates
	@python3 tests/test_template.py all

# Run complete verification: unit tests and template verification
test-all: test-unit test-templates

# Convenience alias for test-unit
test check: test-unit

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
