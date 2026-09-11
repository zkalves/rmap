# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Copyright (c) 2026 Ezequiel Alves. All rights reserved.

.PHONY: all run test check clean clean-gcda clean-coverage rebuild docs docs-serve test-templates test-unit test-backend test-frontend test-all coverage coverage-report install uninstall version bump-patch bump-minor bump-major bump-auto release setup-hooks

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

# Run automated unit tests with compiler code coverage and generate multi-metric summary
coverage:
	@mkdir -p build
	@cd build && cmake .. -DCMAKE_INSTALL_PREFIX="$(PREFIX)" -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON
	@cmake --build build --parallel $(JOBS)
	@rm -rf work
	@mkdir -p work/coverage work/test_templates
	@find build -name "*.gcda" -delete 2>/dev/null || true
	@QT_QPA_PLATFORM=offscreen ctest --test-dir build -L "unit" --output-on-failure
	@python3 tests/test_template.py all
	@python3 script/generate_coverage.py --build-dir build --html work/coverage/index.html --markdown work/coverage/coverage.md --json work/coverage/coverage.json --summary

# Run coverage and update the Code Coverage Metrics report on the GitHub main page (README.md)
coverage-report: coverage
	@python3 script/generate_coverage.py --build-dir build --update-readme

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
