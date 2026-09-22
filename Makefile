# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

.PHONY: all run test check clean clean-gcda clean-coverage rebuild docs docs-pdf docs-user docs-dev docs-classes docs-doxygen docs-serve test-templates test-unit test-backend test-frontend test-examples test-all coverage check-coverage coverage-report install uninstall version bump-patch bump-minor bump-major bump-auto release setup-hooks sim-uvm package-deb package-rpm package-appimage package packages

# Parallel build jobs (defaults to number of processor cores)
JOBS ?= $(shell nproc 2>/dev/null || echo 4)

# Installation prefix and staging directory (override with: make install PREFIX=/custom/path)
PREFIX ?= /usr/local
DESTDIR ?=

# Default target: compile using existing build files
all: build/Makefile
	@cmake --build build --parallel $(JOBS)

# Only run CMake configuration if build/Makefile doesn't exist
build/Makefile: CMakeLists.txt
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX="$(PREFIX)"

# Build only application binary
rmap: build/Makefile
	@cmake --build build --target rmap --parallel $(JOBS)

# Run executable (builds rmap binary only)
run: rmap
	@./build/bin/rmap

# Install rmap executable, templates, examples, and documentation
install: rmap
	@DESTDIR="$(DESTDIR)" cmake --install build --prefix "$(PREFIX)"

# Uninstall rmap executable and installed assets
uninstall:
	@if [ -f build/install_manifest.txt ]; then \
		xargs rm -f < build/install_manifest.txt; \
		rm -rf "$(DESTDIR)$(PREFIX)/share/rmap" 2>/dev/null || true; \
		rm -rf "$(DESTDIR)$(PREFIX)/share/doc/rmap" 2>/dev/null || true; \
		echo "Uninstalled files listed in build/install_manifest.txt"; \
	else \
		rm -f "$(DESTDIR)$(PREFIX)/bin/rmap"; \
		rm -rf "$(DESTDIR)$(PREFIX)/share/rmap" 2>/dev/null || true; \
		rm -rf "$(DESTDIR)$(PREFIX)/share/doc/rmap" 2>/dev/null || true; \
		echo "Uninstalled $(DESTDIR)$(PREFIX)/bin/rmap and assets"; \
	fi

# Package generation targets (DEB, RPM, AppImage)
package-deb: rmap
	@cd build && cpack -G DEB

package-rpm: rmap
	@cd build && cpack -G RPM

package-appimage: rmap
	@./script/build_appimage.sh --build-dir build --output-dir build/packages

packages package: package-deb package-rpm package-appimage

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

# Run comprehensive template verification tests
test-templates: rmap
	@rm -rf work/test_templates
	@mkdir -p work/test_templates
	@python3 tests/test_template.py all

# Run autonomous simulation and compilation across all example environments
test-examples: rmap
	@$(MAKE) -C examples all

# Run UVM verification simulation across example environments (SIM=vcs, SIM=xrun, SIM=mti, UVM_VER=1800.2-2020|1800.2-2017|1.2|1.1d)
sim-uvm: rmap
	@$(MAKE) -C examples sim-uvm SIM=$(SIM) UVM_VER=$(UVM_VER) UVM_HOME=$(UVM_HOME)

# Run complete verification: unit tests, template verification, and all example environments
test-all: test-unit test-templates test-examples

# Run automated unit tests with compiler code coverage and generate multi-metric summary (fails under 100% lines/functions)
coverage check-coverage:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
	@cmake --build build --parallel $(JOBS)
	@rm -rf work
	@mkdir -p work/coverage work/test_templates
	@find build -name "*.gcda" -delete 2>/dev/null || true
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build -L "unit" --output-on-failure
	@python3 tests/test_template.py all
	@python3 script/generate_coverage.py --build-dir build --html work/coverage/index.html --markdown work/coverage/coverage.md --json work/coverage/coverage.json --fail-under-lines 100.0 --fail-under-functions 100.0 --summary

# Run coverage and update coverage badges on the GitHub main page (README.md)
coverage-report: coverage
	@python3 script/generate_coverage.py --build-dir build --fail-under-lines 100.0 --fail-under-functions 100.0 --update-readme

# Convenience alias for test-unit
test check: test-unit

# Clean up profiling counter data (.gcda) to prevent checksum mismatches after code changes
clean-gcda clean-coverage:
	@find build -name "*.gcda" -delete 2>/dev/null || true
	@find . -maxdepth 1 -name "*.gcov*" -delete 2>/dev/null || true
	@rm -rf work/coverage/gcov 2>/dev/null || true
	@echo "Removed all stale .gcda coverage profile files and temporary gcov artifacts."

# Clean up build directory and test artifacts
clean:
	@$(MAKE) -C examples clean 2>/dev/null || true
	@rm -rf build work _site

rebuild: clean all

# Render documentation (HTML portal, User Manual PDF, and Developer Guide PDF)
docs:
	@./script/render_docs

# Generate both User Manual and Developer Guide PDFs
docs-pdf:
	@python3 ./script/generate_docs.py --pdf-only

# Generate User Manual PDF only
docs-user:
	@python3 ./script/generate_docs.py --user-only

# Generate Developer Guide (Doxygen API docs + Developer Guide PDF)
docs-dev: docs-doxygen
	@python3 ./script/generate_docs.py --dev-only

# Generate Doxygen C++ API reference documentation
docs-doxygen:
	@if command -v doxygen >/dev/null 2>&1; then \
		echo "Generating Doxygen C++ API documentation..."; \
		doxygen docs/Doxyfile; \
	else \
		echo "======================================================================"; \
		echo "ERROR: Doxygen is required to generate C++ API documentation,"; \
		echo "       but 'doxygen' was not found on PATH."; \
		echo "======================================================================"; \
		echo "Please install Doxygen:"; \
		echo "  - Ubuntu/Debian: sudo apt-get install -y doxygen graphviz"; \
		echo "  - macOS:         brew install doxygen graphviz"; \
		echo "  - Fedora/RHEL:   sudo dnf install doxygen graphviz"; \
		echo "  - Arch Linux:    sudo pacman -S doxygen graphviz"; \
		exit 1; \
	fi

docs-classes: docs-doxygen


# Serve documentation portal locally
docs-serve:
	@if [ ! -d "_site" ] || [ ! -f "_site/index.html" ]; then \
		echo "Documentation portal not yet generated. Building with 'make docs'..."; \
		$(MAKE) docs; \
	fi
	@echo "Serving documentation portal on http://127.0.0.1:8000 (Ctrl+C to stop)..."; \
	python3 -m http.server 8000 --directory _site


# Semantic Versioning Automation
version:
	@python3 script/bump_version.py --current

bump-patch:
	@python3 script/bump_version.py --patch

bump-minor:
	@python3 script/bump_version.py --minor

bump-major:
	@python3 script/bump_version.py --major

bump-auto:
	@python3 script/bump_version.py --auto

release:
	@python3 script/bump_version.py --auto --tag
	@echo "Release prepared. Push commit and tag with: git push origin main --tags"

setup-hooks:
	@chmod +x script/git-hooks/commit-msg
	@if git config core.hooksPath script/git-hooks 2>/dev/null; then \
		echo "Configured git core.hooksPath -> script/git-hooks"; \
	elif [ -d .git/hooks ] && cp script/git-hooks/commit-msg .git/hooks/commit-msg 2>/dev/null; then \
		chmod +x .git/hooks/commit-msg; \
		echo "Installed Conventional Commits commit-msg hook into .git/hooks/commit-msg"; \
	else \
		echo "Note: .git configuration is read-only in this environment. Run 'git config core.hooksPath script/git-hooks' in your terminal."; \
	fi
