# rmap Developer Guide {#dev_guide}

Welcome to the **rmap** Developer Guide. This guide covers the internal architecture, C++ development environment, coding standards, test infrastructure, and automated Doxygen C++ API reference for developers contributing to or extending **rmap**.

---

## Developer Guide Chapters

- @subpage dev_architecture "C++ Subsystem Architecture"
  Complete architectural walkthrough of the five core subsystems (Controllers, Models, Visualizers, Formats, and Utilities) with data flow diagrams.

- @subpage dev_standards "Code & Documentation Standards"
  Header documentation rules, Modern C++17/Qt 6 conventions, and the Bidirectional Documentation-Implementation Lockstep Invariant.

- @subpage dev_coverage "Code Coverage & Quality Metrics"
  Compiler-level test coverage analysis (Line, Branch, Function, MC/DC Condition), local execution commands, and CI quality gates.

---

## Automated Doxygen C++ API Reference

All class interfaces, method signatures, signals, slots, inheritance hierarchies, and data structures are documented directly within the C++ source headers (`src/` and `src/format/`) using Doxygen docstrings.

- [**C++ Class List (API Reference)**](annotated.html): Complete annotated index of all classes, structs, interfaces, and methods.
- [**Class Inheritance Hierarchy**](hierarchy.html): Inheritance tree and relationship graphs for all models and widgets.
- [**Source Code File List**](files.html): Complete browsable directory of all C++ header and implementation files with syntax-highlighted source code.
- [**Global Functions & Macros**](globals.html): Global functions, enums, type definitions, and preprocessor macros.

### Generating Doxygen Documentation Locally

```bash
# Via project Makefile
make docs-doxygen

# Or directly with doxygen
doxygen docs/Doxyfile
```

The rendered HTML documentation is generated directly into the root documentation portal `_site/`.

> [!IMPORTANT]
> **Prerequisites**: Doxygen is required to build developer API documentation.
> - **Ubuntu/Debian**: `sudo apt-get install -y doxygen graphviz`
> - **macOS**: `brew install doxygen graphviz`
> - **Fedora / RHEL / Rocky / AlmaLinux / CentOS**: `sudo dnf install -y doxygen graphviz`
> - **Arch Linux**: `sudo pacman -S doxygen graphviz`

---

## Development Environment & Build Targets

```bash
# Build rmap with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Execute test suite
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure

# Check architectural invariants and doc-implementation lockstep
python3 script/check_doc_sync.py --static
python3 tests/test_architectural_invariants.py
```

---

## 👤 Author & GitHub Repository

- **Author**: Ezequiel Alves ([@@zkalves](https://github.com/zkalves))
- **GitHub Page**: [https://github.com/zkalves/rmap](https://github.com/zkalves/rmap)
- **Issue Tracker**: [https://github.com/zkalves/rmap/issues](https://github.com/zkalves/rmap/issues)
